#include "RealTimeClock.hpp"
#include "helpers/freertos.hpp"
#include "sync.hpp"

void RealTimeClock::taskMain(void *)
{
    // start up RTC and load values
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

            break;
        }
    }

    syncEventGroup.setBits(sync::RtcHasRespondedOnce);

    // load current time every second
    auto lastWakeTime = xTaskGetTickCount();
    while (true)
    {
        fetchTime();
        manageAlarmEvents();

        vTaskDelayUntil(&lastWakeTime, toOsTicks(1.0_s));
    }
}

//--------------------------------------------------------------------------------------------------
void RealTimeClock::fetchTime()
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
void RealTimeClock::manageAlarmEvents()
{
    if (alarmState != AlarmState::Off)
    {
        // alarm already triggered
        // ToDo: transition sunrise -> vibration -> snooze - vibration -> off
    }
    else if (clockTime.hour == alarmTime1.hour && clockTime.minute == alarmTime1.minute //
             && (alarmMode == AlarmMode::Alarm1 || alarmMode == AlarmMode::Both))
    {
        alarmState = AlarmState::Sunrise;
        // ToDo: implement alarm1 handling
    }
    else if (clockTime.hour == alarmTime2.hour && clockTime.minute == alarmTime2.minute //
             && (alarmMode == AlarmMode::Alarm2 || alarmMode == AlarmMode::Both))
    {
        alarmState = AlarmState::Sunrise;
        // ToDo: implement alarm2 handling
    }
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