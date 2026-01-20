#pragma once

#include "State.hpp"

/// State representing both alarm times
class ShowAlarmTimesState : public State
{
public:
    ShowAlarmTimesState() = default;
    ~ShowAlarmTimesState() override = default;

    void onEnter() override
    {
        // ToDo: set first alarm time
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
            {
                // Todo: set second alarm time or go back to Clock state
            }
            else if (action == util::Button::Action::LongPress)
            {
                // ToDo: get current alarm time to change
                return StateId::ChangeAlarmTimes;
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
    }
};