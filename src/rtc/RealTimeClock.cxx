#include "RealTimeClock.hpp"
#include "helpers/freertos.hpp"
#include "sync.hpp"

void RealTimeClock::taskMain(void *)
{
    setupRtcAndAlarms();
    syncEventGroup.setBits(sync::RtcHasRespondedOnce);

    auto lastWakeTime = xTaskGetTickCount();
    while (true)
    {
        // load current time every second
        fetchClockTime();
        checkIfAlarmShouldTrigger();

        vTaskDelayUntil(&lastWakeTime, toOsTicks(1.0_s));
    }
}

//--------------------------------------------------------------------------------------------------
/// try to initialize RTC until it is online and returns valid time and alarms
void RealTimeClock::setupRtcAndAlarms()
{
    while (true)
    {
        initRTC();

        if (!isRtcOnline())
        {
            vTaskDelay(toOsTicks(1.0_s));
            continue;
        }

        auto timeOptional = rtcModule.getTime();
        auto alarm1Optional = rtcModule.getAlarm1();
        auto alarm2Optional = rtcModule.getAlarm2();

        if (timeOptional && alarm1Optional && alarm2Optional)
        {
            clockTime = timeOptional.value();
            alarmTime1 = alarm1Optional.value();
            alarmTime2 = alarm2Optional.value();
            wasRtcOnlineOnceBool = true;

            // cap alarm minutes to factor 5
            alarmTime1.minute -= (alarmTime1.minute % 5);
            alarmTime2.minute -= (alarmTime2.minute % 5);
            writeAlarmTime1(alarmTime1);
            writeAlarmTime2(alarmTime2);

            return;
        }
    }
}

//--------------------------------------------------------------------------------------------------
void RealTimeClock::fetchClockTime()
{
    auto timeValueOptional = rtcModule.getTime();

    if (timeValueOptional)
        clockTime = timeValueOptional.value();
    else
    {
        // ToDo: error handling (e.g. flag for state machine)

        // increment seconds as fallback
        clockTime.addSeconds(1);
    }
}

//--------------------------------------------------------------------------------------------------
void RealTimeClock::checkIfAlarmShouldTrigger()
{
    if (alarmState != AlarmState::Off)
        return;

    // lambda function to check if give alarm time matchs current time
    auto checkAlarm = [this](Time &&alarmTime)
    { return clockTime.hour == alarmTime.hour && clockTime.minute == alarmTime.minute; };

    static const Time HalfHourBeforeAlarm{"00:30"};
    bool alarm1Triggered = checkAlarm(alarmTime1 - HalfHourBeforeAlarm) &&
                           (alarmMode == AlarmMode::Alarm1 || alarmMode == AlarmMode::Both);

    bool alarm2Triggered = checkAlarm(alarmTime2 - HalfHourBeforeAlarm) &&
                           (alarmMode == AlarmMode::Alarm2 || alarmMode == AlarmMode::Both);

    if (alarm1Triggered || alarm2Triggered)
    {
        if (isAlarmAlreadyTriggered)
            return;

        isAlarmAlreadyTriggered = true;
        alarmState = AlarmState::Sunrise;
    }
    else
        isAlarmAlreadyTriggered = false;
}

//--------------------------------------------------------------------------------------------------
void RealTimeClock::initRTC()
{
    rtcModule.enable32KHz(false);
    rtcModule.setInterruptOutput(true);
    rtcModule.clearAlarm1Flag();
    rtcModule.clearAlarm2Flag();
}

//--------------------------------------------------------------------------------------------------
bool RealTimeClock::isRtcOnline()
{
    return !rtcModule.isCommunicationFailed();
}

//--------------------------------------------------------------------------------------------------
bool RealTimeClock::wasRtcOnlineOnce()
{
    return wasRtcOnlineOnceBool;
}

//--------------------------------------------------------------------------------------------------
Time RealTimeClock::getClockTime() const
{
    return clockTime;
}

//--------------------------------------------------------------------------------------------------
Time RealTimeClock::getAlarmTime1()
{
    auto timeOptional = rtcModule.getAlarm1();
    if (timeOptional)
        alarmTime1 = timeOptional.value();

    return alarmTime1;
}

//--------------------------------------------------------------------------------------------------
Time RealTimeClock::getAlarmTime2()
{
    auto timeOptional = rtcModule.getAlarm2();
    if (timeOptional)
        alarmTime2 = timeOptional.value();

    return alarmTime2;
}

//--------------------------------------------------------------------------------------------------
bool RealTimeClock::writeAlarmTime1(Time &newAlarmTime)
{
    return rtcModule.setAlarm1(newAlarmTime);
}

//--------------------------------------------------------------------------------------------------
bool RealTimeClock::writeAlarmTime2(Time &newAlarmTime)
{
    return rtcModule.setAlarm2(newAlarmTime);
}

//--------------------------------------------------------------------------------------------------
bool RealTimeClock::writeClockTime(Time &newClockTime)
{
    return rtcModule.setTime(newClockTime);
}