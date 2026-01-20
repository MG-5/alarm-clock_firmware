#pragma once

#include "State.hpp"

/// State representing the clock display
class ClockState : public State
{
public:
    ClockState() = default;
    ~ClockState() override = default;

    void onEnter() override
    {
        showAlarmLeds = true;
    }

    void onExit() override;

    std::optional<StateId> update(units::si::Time timePassed) override;

    std::optional<StateId> onButtonEvent(Buttons::ButtonId buttonId, util::Button::Action action) override
    {
        switch (buttonId)
        {
        case Buttons::ButtonId::Left:
        {
            if (action == util::Button::Action::ShortPress)
                return StateId::AlarmTimes;

            else if (action == util::Button::Action::SuperLongPress)
                return StateId::Standby;
        }
        break;

        case Buttons::ButtonId::Right:
        {
            if (action == util::Button::Action::ShortPress)
                return StateId::CurrentAlarm;
        }
        break;

        case Buttons::ButtonId::Snooze:
        {
            if (action == util::Button::Action::ShortPress)
            {
                showAlarmLeds = true;
                // ToDo: reset timer
            }
        }

        default:
            break;
        }

        return std::nullopt;
    }

private:
    bool showAlarmLeds = true;
};