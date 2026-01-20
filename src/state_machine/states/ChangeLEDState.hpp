#pragma once

#include "State.hpp"

class ChangeLEDState : public State
{

public:
    ChangeLEDState() = default;
    ~ChangeLEDState() = default;

    virtual void onEnter() override {};

    virtual void onExit() override {};

    std::optional<StateId> update(units::si::Time timePassed) override
    {
        return std::nullopt;
    }

    std::optional<StateId> onButtonEvent(Buttons::ButtonId buttonId, util::Button::Action action) override
    {
        return std::nullopt;
    }
};