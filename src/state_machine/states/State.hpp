#pragma once

#include "LED/LedStrip.hpp"
#include "LED/StatusLeds.hpp"
#include "display/Display.hpp"
#include "rtc/RealTimeClock.hpp"

#include "state_machine/buttons/Buttons.hpp"
#include <functional>
#include <optional>

enum class StateEvent
{
    Redraw,
    StateChange
};

enum class StateId
{
    Standby,
    Clock,
    AlarmTimes,
    CurrentAlarm,
    ChangeAlarm1,
    ChangeAlarm2,
    ChangeClock,
    ChangeLedStrip,
    Test
};

// callback function
using StateEventCallback = std::function<void(StateEvent event, std::optional<StateId> targetState)>;

struct SystemComponents
{
    Display &display;
    RealTimeClock &rtc;
    StatusLeds &statusLeds;
    LedStrip &ledStrip;
};

/// Base class for all states in the state machine
class State
{
public:
    State(SystemComponents &resources, StateEventCallback &stateEventCallback)
        : systemComponents(resources), //
          stateEventCallback(stateEventCallback) {};

    virtual ~State() = default;

    /// Called when the state is entered
    virtual void onEnter() = 0;

    /// Called when the state is exited
    virtual void onExit() = 0;

    /// Called to draw the current state on the display
    virtual void draw() = 0;

    /// Called when a button event occurs
    virtual std::optional<StateId> onButtonEvent(Buttons::ButtonId buttonId, util::Button::Action action) = 0;

    units::si::Time getUpdateDelay() const
    {
        configASSERT(updateDelay != 0.0_s);
        return updateDelay;
    }

protected:
    void setUpdateDelay(units::si::Time delay)
    {
        updateDelay = delay;
    }

    /// let state machine know that a redraw is requested
    /// this will be done after the end of current function (e.g. draw)
    void requestRedraw()
    {
        stateEventCallback(StateEvent::Redraw, std::nullopt);
    }

    /// let state machine know that a state change is requested
    /// will be done immediately by notifying the state machine task
    void requestStateChange(StateId targetState)
    {
        stateEventCallback(StateEvent::StateChange, targetState);
    }

    units::si::Time updateDelay;

    SystemComponents &systemComponents;
    StateEventCallback &stateEventCallback;
};
