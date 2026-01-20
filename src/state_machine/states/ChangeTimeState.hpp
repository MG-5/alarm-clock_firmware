#pragma once

#include "LED/StatusLeds.hpp"
#include "State.hpp"
#include "rtc/RealTimeClock.hpp"

class ChangeTimeState : public State
{
public:
    enum class TimeToModify
    {
        Alarm1,
        Alarm2,
        Clock
    };

    ChangeTimeState(StatusLeds &statusLeds, TimeToModify timeToModify, RealTimeClock &rtc)
        : statusLeds(statusLeds),     //
          timeToModify(timeToModify), //
          rtc(rtc) {};

    virtual void onEnter() override
    {
        timeType = TimeType::Hour;
        isTimeModified = false;

        targetTime = (timeToModify == TimeToModify::Alarm1)   ? rtc.getAlarmTime1()
                     : (timeToModify == TimeToModify::Alarm2) ? rtc.getAlarmTime2()
                                                              : rtc.getClockTime();
    }

    virtual void onExit() override
    {
        if (!isTimeModified)
            return;

        if (timeToModify == TimeToModify::Alarm1)
        {
            signalResult(rtc.writeAlarmTime1(targetTime));
            rtc.setAlarmMode(RealTimeClock::AlarmMode::Alarm1);
        }
        else if (timeToModify == TimeToModify::Alarm2)
        {
            signalResult(rtc.writeAlarmTime2(targetTime));
            rtc.setAlarmMode(RealTimeClock::AlarmMode::Alarm2);
        }
        else if (timeToModify == TimeToModify::Clock)
            signalResult(rtc.writeClockTime(targetTime));
    }

    std::optional<StateId> update(units::si::Time timePassed) override
    {
        // ToDo: implement blinking display of current changing value
        // ToDo: increment/decrement every 200ms when button is held

        return std::nullopt;
    }

    std::optional<StateId> onButtonEvent(Buttons::ButtonId buttonId, util::Button::Action action) override
    {
        switch (action)
        {
        case util::Button::Action::ShortPress:
        {
            switch (buttonId)
            {
            case Buttons::ButtonId::Left:
            {
                switch (timeType)
                {
                case TimeType::Hour:
                    timeType = TimeType::Minute;
                    break;

                case TimeType::Minute:
                    return StateId::Clock;
                    break;
                }
            }
            break;

            case Buttons::ButtonId::Right:
            case Buttons::ButtonId::BrightnessPlus:
            case Buttons::ButtonId::CCTPlus:
                incrementNumber();
                break;

            case Buttons::ButtonId::Snooze:
            {
                // abort change and go back to clock
                isTimeModified = false;
                return StateId::Clock;
            }
            break;

            case Buttons::ButtonId::BrightnessMinus:
            case Buttons::ButtonId::CCTMinus:
                decrementNumber();
                break;
            }
        }

        case util::Button::Action::LongPress:
        {
            // start increment/decrement continuously
            isContinuousChange = true;

            if (buttonId == Buttons::ButtonId::Right || buttonId == Buttons::ButtonId::BrightnessPlus ||
                buttonId == Buttons::ButtonId::CCTPlus)
            {
                isIncrementing = true;
                incrementNumber();
            }
            else if (buttonId == Buttons::ButtonId::BrightnessMinus || buttonId == Buttons::ButtonId::CCTMinus)
            {
                isIncrementing = false;
                decrementNumber();
            }
        }
        break;

        case util::Button::Action::StopLongPress:
            isContinuousChange = false;
            break;

        default:
            break;
        }

        return std::nullopt;
    }

private:
    enum class TimeType
    {
        Hour,
        Minute
    } timeType = TimeType::Hour;

    StatusLeds &statusLeds;
    TimeToModify timeToModify;
    RealTimeClock &rtc;

    Time targetTime;

    bool isTimeModified = false;

    bool isContinuousChange = false;
    bool isIncrementing = true;

    void signalResult(bool success)
    {
        success ? statusLeds.signalSuccess() : statusLeds.signalError();
    }

    void incrementNumber()
    {
        isTimeModified = true;
        if (timeType == TimeType::Hour)
            targetTime.addHours(1);

        else
            targetTime.addMinutes(timeToModify == TimeToModify::Clock ? 1 : 5);
    }

    void decrementNumber()
    {
        isTimeModified = true;
        if (timeType == TimeType::Hour)
            targetTime.subHours(1);

        else
            targetTime.subMinutes(timeToModify == TimeToModify::Clock ? 1 : 5);
    }
};