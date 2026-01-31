#include "StateMachine.hpp"

void StateMachine::taskMain(void *parameters)
{
    waitForRtc();
    displayLedInitialization();

    while (true)
    {
        systemComponents.display.clearGridDataArray();
        systemComponents.statusLeds.turnOffAlarmLeds();
        stateChanged = false;

        currentState->onEnter();

        while (true)
        {
            currentState->draw();

            if (stateChanged || !delayUntilEventOrTimeout(currentState->getUpdateDelay()))
            {
                if (shouldRedraw)
                {
                    shouldRedraw = false;
                    continue; // redraw requested, continue loop
                }
                break; // state changed/event occurred, exit loop to process it
            }

            if (stateChanged)
                break; // state changed, exit loop to process it
        }

        currentState->onExit();
        currentState = getStateFromId(currentStateId);
    }
}

// -----------------------------------------------------------------
State *StateMachine::getStateFromId(StateId stateId)
{
    switch (stateId)
    {
    case StateId::Standby:
        return &standbyState;

    case StateId::AlarmTimes:
        return &alarmTimesState;

    case StateId::ChangeClock:
        return &changeTimeState;

    case StateId::ChangeAlarm1:
        return &changeAlarm1State;

    case StateId::ChangeAlarm2:
        return &changeAlarm2State;

    case StateId::CurrentAlarm:
        return &showCurrentAlarmMode;

    case StateId::ChangeLedStrip:
        return &changeLEDState;

    case StateId::Test:
        return &testState;

    case StateId::Clock:
    default:
        return &clockState;
    }
}

// -----------------------------------------------------------------
void StateMachine::commonButtonCallback(Buttons::ButtonId buttonId, util::Button::Action action)
{
    // ToDo: check for alarm

    if (buttonId == Buttons::ButtonId::Snooze)
    {
        if (action == util::Button::Action::ShortPress)
        {
            requestStateChange(StateId::Clock);
        }
        else if (action == util::Button::Action::LongPress)
        {
            isLedStripOn = !isLedStripOn;
            isLedStripOn ? systemComponents.ledStrip.turnOnWithFade() : systemComponents.ledStrip.turnOffWithFade();

            if (isLedStripOn)
            {
                if (currentStateId == StateId::Standby)
                {
                    // if we are in standby, turn on the display to show the change immediately
                    requestStateChange(StateId::Clock);
                }
            }
            else if (currentStateId == StateId::Clock)
            {
                // if we are in clock state, check if we should go to standby after turning off the led strip
                clockState.evaluateStandbyCondition();
            }
        }
        return;
    }

    // delegate button event to current state and obtain a possibly new state as a result
    auto newStateId = currentState->onButtonEvent(buttonId, action);

    if (newStateId.has_value() && newStateId.value() != currentStateId)
    {
        // state change requested
        requestStateChange(newStateId.value());
        abortDelayWaiting();
    }
}

// -----------------------------------------------------------------
void StateMachine::handleStateEvent(StateEvent event, std::optional<StateId> targetState)
{
    switch (event)
    {
    case StateEvent::Redraw:
        requestRedraw();
        break;

    case StateEvent::StateChange:
        if (targetState.has_value())
            requestStateChange(targetState.value());

        break;
    }

    abortDelayWaiting();
}

// -----------------------------------------------------------------
void StateMachine::assignButtonCallbacks()
{
    // use lambda
    buttons.left.setCallback([this](util::Button::Action action)
                             { commonButtonCallback(Buttons::ButtonId::Left, action); });

    buttons.right.setCallback([this](util::Button::Action action)
                              { commonButtonCallback(Buttons::ButtonId::Right, action); });

    buttons.snooze.setCallback([this](util::Button::Action action)
                               { commonButtonCallback(Buttons::ButtonId::Snooze, action); });

    buttons.brightnessPlus.setCallback([this](util::Button::Action action)
                                       { commonButtonCallback(Buttons::ButtonId::BrightnessPlus, action); });

    buttons.brightnessMinus.setCallback([this](util::Button::Action action)
                                        { commonButtonCallback(Buttons::ButtonId::BrightnessMinus, action); });

    buttons.cctPlus.setCallback([this](util::Button::Action action)
                                { commonButtonCallback(Buttons::ButtonId::CCTPlus, action); });

    buttons.cctMinus.setCallback([this](util::Button::Action action)
                                 { commonButtonCallback(Buttons::ButtonId::CCTMinus, action); });
}

bool StateMachine::delayUntilEventOrTimeout(units::si::Time blockTime)
{
    clearNotifications();
    return !notifyWait(ULONG_MAX, ULONG_MAX, (uint32_t *)0, toOsTicks(blockTime));
}

//-----------------------------------------------------------------
void StateMachine::waitForRtc()
{
    systemComponents.statusLeds.ledRedGreen.setColorBlinking(util::led::pwm::DualLedColor::Red, 2.0_Hz);
    syncEventGroup.waitBits(sync::RtcHasRespondedOnce, pdFALSE, pdFALSE, portMAX_DELAY);
    systemComponents.statusLeds.ledRedGreen.turnOff();
}

//-----------------------------------------------------------------
void StateMachine::displayLedInitialization()
{
    auto &display = systemComponents.display;
    auto &statusLeds = systemComponents.statusLeds;

    statusLeds.turnAllOff();
    display.setup();
    display.showInitialization();
    statusLeds.ledRedGreen.setColor(util::led::pwm::DualLedColor::Orange);
    statusLeds.turnAllOn();
    // vibrationCushion.write(true);
    vTaskDelay(toOsTicks(1.0_s));
    statusLeds.turnAllOff();
    // vibrationCushion.write(false);

    display.enableDisplay(); // start multiplexing
}