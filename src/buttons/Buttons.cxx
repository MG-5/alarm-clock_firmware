#include "Buttons.hpp"

[[noreturn]] void Buttons::taskMain(void *)
{
    auto lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        static constexpr auto ButtonSamplingInterval = 10.0_ms;

        left.update(ButtonSamplingInterval);
        right.update(ButtonSamplingInterval);
        snooze.update(ButtonSamplingInterval);
        brightnessPlus.update(ButtonSamplingInterval);
        brightnessMinus.update(ButtonSamplingInterval);
        cctPlus.update(ButtonSamplingInterval);
        cctMinus.update(ButtonSamplingInterval);

        vTaskDelayUntil(&lastWakeTime, toOsTicks(ButtonSamplingInterval));
    }
}