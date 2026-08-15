#pragma once

#include "application/ui/UiStateMachine.hpp"

#include "hardware/LED/LedStrip.hpp"
#include "hardware/LED/StatusLeds.hpp"
#include "hardware/display/Display.hpp"
#include "hardware/rtc/RealTimeClock.hpp"

/// The entry point of users C++ firmware. This comes after CubeHAL and FreeRTOS initialization.
/// All needed classes and objects have the root here.
class Firmware
{
public:
    static constexpr auto MultiplexingPwmTimer = &htim16;
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

    Firmware();
    [[noreturn]] void run();

    static Firmware &getApplicationInstance();

    static void multiplexingTimerUpdate();
    static void pwmTimerCompare();

private:
    static inline Firmware *instance{nullptr};

    void registerCallbacks();

    DisplayDimming dimming{MultiplexingPwmTimer, PwmTimChannel};
    Display display{dimming};

    StatusLeds statusLeds{StatusLedPwmTimer, LedAlarm1Channel, LedAlarm2Channel, LedRedChannel, LedGreenChannel};
    LedStrip ledStrip{LedStripPwmTimer, WarmWhiteChannel, ColdWhiteChannel};

    I2cAccessor i2cBusAccessor{RtcBus};
    RealTimeClock rtc{i2cBusAccessor};

    SystemComponents systemComponents{display, rtc, statusLeds, ledStrip};
    UiStateMachine uiStateMachine{systemComponents};
};
