#pragma once

#include "LED/LedStrip.hpp"
#include "LED/StatusLeds.hpp"
#include "buttons/Buttons.hpp"
#include "display/Display.hpp"
#include "rtc/RealTimeClock.hpp"
#include "state_machine/StateMachine.hpp"

/// The entry point of users C++ firmware. This comes after CubeHAL and FreeRTOS initialization.
/// All needed classes and objects have the root here.
class Application
{
public:
    static constexpr auto MultiplexingPwmTimer = &htim1;
    static constexpr auto PwmTimChannel = TIM_CHANNEL_1;

    static constexpr auto StatusLedPwmTimer = &htim2;
    static constexpr auto LedAlarm1Channel = TIM_CHANNEL_1;
    static constexpr auto LedAlarm2Channel = TIM_CHANNEL_2;
    static constexpr auto LedRedChannel = TIM_CHANNEL_3;
    static constexpr auto LedGreenChannel = TIM_CHANNEL_4;

    static constexpr auto LedStripPwmTimer = &htim15;
    static constexpr auto WarmWhiteChannel = TIM_CHANNEL_1;
    static constexpr auto ColdWhiteChannel = TIM_CHANNEL_2;

    static constexpr auto RtcBus = &hi2c1;

    Application();
    [[noreturn]] void run();

    static Application &getApplicationInstance();

    static void multiplexingTimerUpdate();
    static void pwmTimerCompare();
    static void statusLedsTimeoutCallback(TimerHandle_t timer);
    static void stateMachineTimeoutCallback(TimerHandle_t timer);

private:
    static inline Application *instance{nullptr};

    void registerCallbacks();

    DisplayDimming dimming{MultiplexingPwmTimer, PwmTimChannel};
    Display display{dimming};

    StatusLeds statusLeds{StatusLedPwmTimer, LedAlarm1Channel, LedAlarm2Channel,
                          LedRedChannel,     LedGreenChannel,  statusLedsTimeoutCallback};
    LedStrip ledStrip{LedStripPwmTimer, WarmWhiteChannel, ColdWhiteChannel};

    Buttons buttons{};

    I2cAccessor i2cBusAccessor{RtcBus};
    RealTimeClock rtc{i2cBusAccessor};

    StateMachine stateMachine{display, statusLeds, ledStrip, buttons, rtc, &stateMachineTimeoutCallback};
};
