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
        determineAlarmTriggerState();

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
            vTaskDelay(toOsTicks(250.0_ms));
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
// check if alarm should be triggered based on current time/alarm times/alarm mode and update alarm state accordingly
void RealTimeClock::determineAlarmTriggerState()
{
    if (alarmMode == AlarmMode::Off)
        return;

    // lambda function to check if given alarm time matchs current time
    auto checkAlarm = [this](Time &&alarmTime)
    { return clockTime.hour == alarmTime.hour && clockTime.minute == alarmTime.minute; };

    // half hour before alarm time to trigger sunrise
    static const Time HalfHour{"00:30"};
    bool isAlarm1SunriseTriggered =
        checkAlarm(alarmTime1 - HalfHour) && (alarmMode == AlarmMode::Alarm1 || alarmMode == AlarmMode::Both);

    bool isAlarm2SunriseTriggered =
        checkAlarm(alarmTime2 - HalfHour) && (alarmMode == AlarmMode::Alarm2 || alarmMode == AlarmMode::Both);

    // only trigger sunrise if alarm is off and not already triggered
    if (alarmState == AlarmState::Off && (isAlarm1SunriseTriggered || isAlarm2SunriseTriggered))
    {
        // start sunrise
        alarmState = AlarmState::Sunrise;

        // ToDo: handle transition, reset led strip and counter for sunrise effect
        return;
    }

    // alarm time to trigger vibration
    bool isAlarm1VibrationTriggered =
        checkAlarm(std::move(alarmTime1)) && (alarmMode == AlarmMode::Alarm1 || alarmMode == AlarmMode::Both);

    bool isAlarm2VibrationTriggered =
        checkAlarm(std::move(alarmTime2)) && (alarmMode == AlarmMode::Alarm2 || alarmMode == AlarmMode::Both);

    // only trigger vibration if sunrise is running
    if (alarmState == AlarmState::Sunrise && (isAlarm1VibrationTriggered || isAlarm2VibrationTriggered))
    {
        alarmState = AlarmState::Vibration;

        // ToDo: handle transition, reset vibration state
        return;
    }

    if (alarmState != AlarmState::Snooze)

        return;

    constexpr auto SnoozeTime = 5;

    bool isAlarm1SnoozeTriggered = (clockTime != alarmTime1) &&
                                   (Time::getDifferenceInMinutes(clockTime, alarmTime1) % SnoozeTime == 0) &&
                                   (alarmMode == AlarmMode::Alarm1 || alarmMode == AlarmMode::Both);

    bool isAlarm2SnoozeTriggered = (clockTime != alarmTime2) &&
                                   (Time::getDifferenceInMinutes(clockTime, alarmTime2) % SnoozeTime == 0) &&
                                   (alarmMode == AlarmMode::Alarm2 || alarmMode == AlarmMode::Both);

    if (isAlarm1SnoozeTriggered || isAlarm2SnoozeTriggered)
        alarmState = AlarmState::Vibration;
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

//--------------------------------------------------------------------------------------------------
void RealTimeClock::handleButtonEvents(Buttons::ButtonId buttonId, util::Button::Action action)
{
    // from all states alarm can be turned off by long pressing the left button
    // sunrise/snooze -> no button interaction possible
    // vibration -> snooze button

    if (buttonId == Buttons::ButtonId::Left && action == util::Button::Action::LongPress)
    {
        alarmState = AlarmState::Off;
        // ToDo: handle transition?
    }
    else if (alarmState == AlarmState::Vibration && buttonId == Buttons::ButtonId::Snooze &&
             action == util::Button::Action::ShortPress)
    {
        alarmState = AlarmState::Snooze;
    }
}

// --------------------------------------------------------------------------------------------------
void RealTimeClock::processAlarmLogic()
{
    if (alarmState == AlarmState::Off)
        return;

    if (alarmState == AlarmState::Sunrise)
    {
        // ToDo: implement sunrise effect by increasing brightness of led strip in a loop until max brightness is
        // reached or alarm state changes first ramp warm white, then cold white 1024 steps, warmwhite increment every 1
        // second by one step -> 17 minutes ramp time then cold white increment every 1 second by two step -> 8.5
        // minutes ramp time total ramp time 25.5 minutes

        if (sunriseCounter < LedStrip::PwmSteps)
        {
            sys ledStrip.setRawWarmWhiteLevel(sunriseCounter);
            sunriseCounter++;
        }
        else if (sunriseCounter < 3 * LedStrip::PwmSteps)
        {
            ledStrip.setRawColdWhiteLevel((sunriseCounter - LedStrip::PwmSteps) * 2);
            sunriseCounter++;
        }
    }
}