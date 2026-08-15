#pragma once

#include "State.hpp"

class ChangeLEDState : public State
{

public:
    ChangeLEDState(SystemComponents &systemComponents, StateEventCallback &stateEventCallback)
        : State(systemComponents, stateEventCallback) {};
    ~ChangeLEDState() override = default;

    //-----------------------------------------------------------------
    void onEnter() override
    {
    }

    //-----------------------------------------------------------------
    void onExit() override
    {
    }

    //-----------------------------------------------------------------
    void draw() override
    {
        // ToDo: show current LED settings on display
    }

    //-----------------------------------------------------------------
    std::optional<StateId> onButtonEvent(Buttons::ButtonId buttonId, util::Button::Action action) override
    {
        // ToDo: set LED settings based on button events
        // ToDo: reset timeout
        return std::nullopt;
    }
};