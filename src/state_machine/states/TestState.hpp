#pragma once
#include "State.hpp"

class TestState : public State
{
public:
    TestState(SystemComponents &systemComponents, StateEventCallback &stateEventCallback)
        : State(systemComponents, stateEventCallback) {};
    ~TestState() override = default;

    //-----------------------------------------------------------------
    void onEnter() override
    {
        // ToDo: turn on vibration cushion
        // ToDo: set LED strip to test pattern
        // ToDo: show all segments on display
    }

    //-----------------------------------------------------------------
    void onExit() override
    {
        // ToDo: turn off vibration cushion
        // ToDo: set previous LED strip state
    }

    //-----------------------------------------------------------------
    void draw() override
    {
        // ToDo: ?
        setUpdateDelay(1.0_s);
    }

    //-----------------------------------------------------------------
    std::optional<StateId> onButtonEvent(Buttons::ButtonId buttonId, util::Button::Action action) override
    {
        return std::nullopt;
    }
};