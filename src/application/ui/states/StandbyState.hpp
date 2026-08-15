#pragma once

#include "State.hpp"

/// State representing the standby mode
class StandbyState : public State
{
public:
    StandbyState(SystemComponents &resources, StateEventCallback &stateEventCallback)
        : State(resources, stateEventCallback) {};

    ~StandbyState() override = default;

    //-----------------------------------------------------------------
    void onEnter() override
    {
        systemComponents.display.disableDisplay();
    }

    //-----------------------------------------------------------------
    void onExit() override
    {
        systemComponents.display.enableDisplay();
    }

    //-----------------------------------------------------------------
    void draw() override
    {
        // Nothing to draw in standby
        setUpdateDelay(1.0_min);
    }

    //-----------------------------------------------------------------
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
};