#pragma once

#include "State.hpp"

/// State representing both alarm times
class ShowAlarmTimesState : public State
{
public:
    ShowAlarmTimesState(SystemComponents &systemComponents, StateEventCallback &stateEventCallback)
        : State(systemComponents, stateEventCallback) {};
    ~ShowAlarmTimesState() override = default;

    //-----------------------------------------------------------------
    void onEnter() override
    {
        currentAlarmTime = AlarmTime::Alarm1;
        blink = true;
    }

    //-----------------------------------------------------------------
    void onExit() override
    {
    }

    //-----------------------------------------------------------------
    void draw() override
    {
        auto &display = systemComponents.display;
        auto &rtc = systemComponents.rtc;
        auto &statusLeds = systemComponents.statusLeds;
        statusLeds.turnOffAlarmLeds();

        if (currentAlarmTime == AlarmTime::Alarm1)
        {
            display.setClock(rtc.getAlarmTime1());
            statusLeds.ledAlarm1.setState(blink);
        }
        else
        {
            display.setClock(rtc.getAlarmTime2());
            statusLeds.ledAlarm2.setState(blink);
        }

        blink = !blink;
        display.renderClock(true);
        setUpdateDelay(500.0_ms);
    }

    //-----------------------------------------------------------------
    std::optional<StateId> onButtonEvent(Buttons::ButtonId buttonId, util::Button::Action action) override
    {
        switch (buttonId)
        {
        case Buttons::ButtonId::Left:
        {
            if (action == util::Button::Action::ShortPress)
            {
                requestRedraw();
                blink = true;

                if (currentAlarmTime == AlarmTime::Alarm1)
                    currentAlarmTime = AlarmTime::Alarm2;

                else
                    return StateId::Clock;
            }
            else if (action == util::Button::Action::LongPress)
            {
                if (currentAlarmTime == AlarmTime::Alarm1)
                    return StateId::ChangeAlarm1;

                else
                    return StateId::ChangeAlarm2;
            }
        }
        break;

        case Buttons::ButtonId::Right:
        {
            if (action == util::Button::Action::ShortPress)
                return StateId::CurrentAlarm;

            else if (action == util::Button::Action::SuperLongPress)
                return StateId::ChangeClock;
        }
        break;

        default:
            break;
        }

        return std::nullopt;
    }

private:
    enum class AlarmTime
    {
        Alarm1,
        Alarm2
    } currentAlarmTime = AlarmTime::Alarm1;

    bool blink = true;
};