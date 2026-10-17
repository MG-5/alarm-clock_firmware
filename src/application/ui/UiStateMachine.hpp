#pragma once

#include <climits>

#include "states/ChangeLEDState.hpp"
#include "states/ChangeTimeState.hpp"
#include "states/ClockState.hpp"
#include "states/ShowAlarmTimesState.hpp"
#include "states/ShowCurrentAlarmMode.hpp"
#include "states/StandbyState.hpp"
#include "states/TestState.hpp"

#include "hardware/display/Display.hpp"
#include "sync.hpp"

class UiStateMachine : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    UiStateMachine(Display &display, RealTimeClock &rtc, StatusLeds &statusLeds, LedStrip &ledStrip)
        : util::wrappers::TaskWithMemberFunctionBase("uiStateMachineTask", 512, osPriorityBelowNormal4), //
          systemComponents({display, rtc, statusLeds, ledStrip})
    {
    }

    ~UiStateMachine() = default;

    void requestStateChange(StateId newStateId)
    {
        currentStateId = newStateId;
        stateChanged = true;

        abortDelayWaiting();
    }

    void handleStateEvent(StateEvent event, std::optional<StateId> targetState);
    void handleButtonEvent(Buttons::ButtonId buttonId, util::Button::Action action);

protected:
    void taskMain(void *parameters) override;

private:
    SystemComponents systemComponents;

    StateEventCallback stateEventCallback{
        std::bind(&UiStateMachine::handleStateEvent, this, std::placeholders::_1, std::placeholders::_2)};

    StandbyState standbyState{systemComponents, stateEventCallback};
    ClockState clockState{systemComponents, stateEventCallback};
    ShowAlarmTimesState alarmTimesState{systemComponents, stateEventCallback};
    ChangeTimeState changeTimeState{systemComponents, ChangeTimeState::TimeToModify::Clock, stateEventCallback};
    ChangeTimeState changeAlarm1State{systemComponents, ChangeTimeState::TimeToModify::Alarm1, stateEventCallback};
    ChangeTimeState changeAlarm2State{systemComponents, ChangeTimeState::TimeToModify::Alarm2, stateEventCallback};
    ShowCurrentAlarmMode showCurrentAlarmMode{systemComponents, stateEventCallback};
    ChangeLEDState changeLEDState{systemComponents, stateEventCallback};
    TestState testState{systemComponents, stateEventCallback};

    State *currentState = &clockState;
    StateId currentStateId = StateId::Clock;
    bool stateChanged = false;
    bool shouldRedraw = false;

    bool isLedStripOn = false;

    State *getStateFromId(StateId stateId);

    /// block task for specified time but can be unblocked by external event e.g. button press
    /// @return true if timeout is occurred
    bool delayUntilEventOrTimeout(units::si::Time blockTime);

    void waitForRtc();

    void displayLedInitialization();

    // abort delayUntilEventOrTimeout function by notifying the task
    void abortDelayWaiting()
    {
        notifyGive();
    }
};
