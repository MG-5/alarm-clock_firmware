#pragma once

#include "state_machine/buttons/Buttons.hpp"
#include <optional>

/// Base class for all states in the state machine
class State
{
public:
    enum class StateId
    {
        Standby,
        Clock,
        AlarmTimes,
        CurrentAlarm,
        ChangeAlarmTimes,
        ChangeClock,
        ChangeLedStrip,
        Test
    };

    virtual ~State() = default;

    /// Called when the state is entered
    virtual void onEnter() = 0;

    /// Called when the state is exited
    virtual void onExit() = 0;

    /// Called periodically to update the state
    virtual std::optional<StateId> update(units::si::Time timePassed) = 0;

    /// Called when a button event occurs
    virtual std::optional<StateId> onButtonEvent(Buttons::ButtonId buttonId, util::Button::Action action) = 0;
};
