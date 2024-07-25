#pragma once

#include "Display.hpp"
#include "LED/StatusLeds.hpp"
#include "buttons/Buttons.hpp"
#include "util/gpio.hpp"
#include "wrappers/Task.hpp"

#include <climits>

class StateMachine : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    StateMachine(Display &display, StatusLeds &statusLeds, Buttons &buttons)
        : TaskWithMemberFunctionBase("stateMachineTask", 256, osPriorityNormal4), //
          display(display),                                                       //
          statusLeds(statusLeds),                                                 //
          buttons(buttons)
    {
        setButtonCallbacks();
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
        Vibration
    };

    enum class AlarmMode
    {
        Off,
        Alarm1,
        Alarm2,
        Both
    };

    void updateClock(Time &newTime)
    {
        clockTime = newTime;
    }

protected:
    void taskMain(void *) override;

private:
    Display &display;
    StatusLeds &statusLeds;
    Buttons &buttons;

    Time clockTime;
    Time alarmTime1{11, 00};
    Time alarmTime2{10, 30};

    DisplayState displayState = DisplayState::Clock;
    AlarmState alarmState = AlarmState::Off;
    AlarmMode alarmMode = AlarmMode::Both;
    bool blink = true;

    void
    handleAlarmHourChange(bool blink, Time &alarmTime,
                          util::pwm_led::SingleLed<StatusLeds::NumberOfResolutionBits> &ledAlarm);
    void
    handleAlarmMinuteChange(bool blink, Time &alarmTime,
                            util::pwm_led::SingleLed<StatusLeds::NumberOfResolutionBits> &ledAlarm);

    void setButtonCallbacks();

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