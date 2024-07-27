#pragma once

#include "LED/StatusLeds.hpp"
#include "buttons/Buttons.hpp"
#include "display/Display.hpp"
#include "rtc/RealTimeClock.hpp"

#include "util/gpio.hpp"
#include "wrappers/Task.hpp"

#include <climits>

class StateMachine : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    StateMachine(Display &display, StatusLeds &statusLeds, Buttons &buttons, RealTimeClock &rtc)
        : TaskWithMemberFunctionBase("stateMachineTask", 512, osPriorityNormal4), //
          display(display),                                                       //
          statusLeds(statusLeds),                                                 //
          buttons(buttons),                                                       //
          rtc(rtc)
    {
        assignButtonCallbacks();
    }

    enum class DisplayState
    {
        Standby,
        Clock,
        ClockWithAlarmLeds,
        DisplayAlarm1,
        DisplayAlarm2,
        ChangeAlarm1Hour,
        ChangeAlarm1Minute,
        ChangeAlarm2Hour,
        ChangeAlarm2Minute,
        DisplayAlarmStatus,
        LedBrightness,
        LedCCT,
        Test
    };

    enum class AlarmState
    {
        Off,
        Sunrise,
        Vibration,
        Snooze
    };

    enum class AlarmMode
    {
        Off,
        Alarm1,
        Alarm2,
        Both
    };

protected:
    void taskMain(void *) override;

private:
    Display &display;
    StatusLeds &statusLeds;
    Buttons &buttons;
    RealTimeClock &rtc;

    DisplayState displayState = DisplayState::Clock;
    AlarmState alarmState = AlarmState::Off;
    AlarmMode alarmMode = AlarmMode::Both;
    bool blink = true;

    Time alarmTimeToModify;

    void displayLedInitialization();
    void waitForRtc();

    void handleAlarmHourChange(StatusLeds::SingleLed &ledAlarm);
    void handleAlarmMinuteChange(StatusLeds::SingleLed &ledAlarm);

    void showCurrentAlarmMode();

    void assignButtonCallbacks();

    void buttonLeftCallback(util::Button::Action action);
    void buttonRightCallback(util::Button::Action action);
    void buttonSnoozeCallback(util::Button::Action action);
    void buttonBrightnessPlusCallback(util::Button::Action action);
    void buttonBrightnessMinusCallback(util::Button::Action action);
    void buttonCCTPlusCallback(util::Button::Action action);
    void buttonCCTMinusCallback(util::Button::Action action);

    /// block task for specified time but can be unblocked by external event e.g. button press
    /// @return true if timeout is occurred
    bool delayUntilEventOrTimeout(units::si::Time blockTime)
    {
        return notifyWait(ULONG_MAX, ULONG_MAX, (uint32_t *)0, toOsTicks(blockTime)) == 0;
    }
};