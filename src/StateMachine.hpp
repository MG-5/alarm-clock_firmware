#pragma once

#include "Display.hpp"
#include "LED/StatusLeds.hpp"
#include "buttons/Buttons.hpp"
#include "util/gpio.hpp"
#include "wrappers/Task.hpp"

class StateMachine : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    StateMachine(Display &display, StatusLeds &statusLeds)
        : TaskWithMemberFunctionBase("stateMachineTask", 256, osPriorityNormal4), //
          display(display),                                                       //
          statusLeds(statusLeds){};

    enum class State
    {
        Standby,
        Clock,
        DisplayAlarm1,
        DisplayAlarm2,
        ChangeAlarm1Hour,
        ChangeAlarm1Minute,
        ChangeAlarm2Hour,
        ChangeAlarm2Minute,
        DisplayAlarmStatus,
        LedBrightness,
        LedCCT
    };

protected:
    void taskMain(void *) override;

private:
    Display &display;
    StatusLeds &statusLeds;

    State state = State::Clock;

    void
    handleAlarmHourChange(bool blink, Clock_t &alarmTime,
                          util::pwm_led::SingleLed<StatusLeds::NumberOfResolutionBits> &ledAlarm);
    void
    handleAlarmMinuteChange(bool blink, Clock_t &alarmTime,
                            util::pwm_led::SingleLed<StatusLeds::NumberOfResolutionBits> &ledAlarm);
};