#pragma once

#include "Display.hpp"
#include "StateMachine.hpp"
#include "buttons/Buttons.hpp"

/// The entry point of users C++ firmware. This comes after CubeHAL and FreeRTOS initialization.
/// All needed classes and objects have the root here.
class Application
{
public:
    static constexpr auto MultiplexingPwmTimer = &htim1;
    static constexpr auto PwmTimChannel = TIM_CHANNEL_1;

    static constexpr auto LedPwmTimer = &htim2;
    static constexpr auto LedAlarm1Channel = TIM_CHANNEL_1;
    static constexpr auto LedAlarm2Channel = TIM_CHANNEL_2;
    static constexpr auto LedRedChannel = TIM_CHANNEL_3;
    static constexpr auto LedGreenChannel = TIM_CHANNEL_4;

    Application();
    [[noreturn]] void run();

    static Application &getApplicationInstance();

    static void multiplexingTimerUpdate();
    static void pwmTimerCompare();

private:
    static inline Application *instance{nullptr};

    void registerCallbacks();

    Buttons buttons{};
    Dimming dimming{MultiplexingPwmTimer, PwmTimChannel};
    Display display{dimming};

    StatusLeds statusLeds{LedPwmTimer, LedAlarm1Channel, LedAlarm2Channel, LedRedChannel,
                          LedGreenChannel};
    StateMachine stateMachine{display, statusLeds, buttons};
};
