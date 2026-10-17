#pragma once

#include "Buttons.hpp"

#include "application/ui/UiStateMachine.hpp"
#include "application/wakeup/WakeupController.hpp"

class InputRouter
{
public:
    InputRouter(Buttons &buttons, UiStateMachine &uiStateMachine, WakeupController &wakeupController)
        : buttons(buttons), uiStateMachine(uiStateMachine), wakeupController(wakeupController)
    {
        assignButtonCallbacks();
    }

private:
    Buttons &buttons;
    UiStateMachine &uiStateMachine;
    WakeupController &wakeupController;

    void assignButtonCallbacks()
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

    void commonButtonCallback(Buttons::ButtonId buttonId, util::Button::Action action)
    {
        if (wakeupController.isActive())
            // delegate button event to wakeup controller
            wakeupController.handleButtonEvent(buttonId, action);
            
        else
            // delegate button event to UI controller
            uiStateMachine.handleButtonEvent(buttonId, action);
    }
};