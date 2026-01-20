#pragma once

#include "State.hpp"
#include "display/Display.hpp"

/// State representing the standby mode
class StandbyState : public State
{
public:
    StandbyState(Display &display) : display(display)
    {
    }

    ~StandbyState() override = default;

    void onEnter() override
    {
        display.disableDisplay();
    }

    void onExit() override
    {
        display.enableDisplay();
    }

    std::optional<StateId> update(units::si::Time timePassed) override;
    std::optional<StateId> onButtonEvent(Buttons::ButtonId buttonId, util ::Button::Action action) override
    {
        switch (buttonId)
        {
        case Buttons::ButtonId::Left:
        case Buttons::ButtonId::Right:
        case Buttons::ButtonId::Snooze:

            if (action == util::Button::Action::ShortPress)
                return StateId::Clock;

            break;

        default:
            break;
        }

        return std::nullopt;
    }

private:
    Display &display;
};