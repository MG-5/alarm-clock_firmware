#pragma once

#include "tim.h"
#include "util/PwmLed.hpp"
#include "wrappers/Task.hpp"
#include <bit>

class StatusLeds : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    StatusLeds(TIM_HandleTypeDef *ledTimerHandle, uint32_t ledAlarm1Channel,
               uint32_t ledAlarm2Channel, uint32_t ledRedChannel, uint32_t ledGreenChannel)
        : TaskWithMemberFunctionBase("statusLedTask", 128, osPriorityLow2),
          ledTimerHandle(ledTimerHandle),     //
          ledAlarm1Channel(ledAlarm1Channel), //
          ledAlarm2Channel(ledAlarm2Channel), //
          ledRedChannel(ledRedChannel),       //
          ledGreenChannel(ledGreenChannel)    //
    {
        SafeAssert(this->ledTimerHandle != nullptr);
    }

protected:
    [[noreturn]] void taskMain(void *)
    {
        ledAlarm1.startPwmTimer();
        ledAlarm2.startPwmTimer();
        ledRedGreen.startPwmTimer();

        auto lastWakeTime = xTaskGetTickCount();

        while (true)
        {
            ledAlarm1.updateState(lastWakeTime);
            ledAlarm2.updateState(lastWakeTime);
            ledRedGreen.updateState(lastWakeTime);
            vTaskDelayUntil(&lastWakeTime, toOsTicks(20.0_Hz));
        }
    }

private:
    TIM_HandleTypeDef *ledTimerHandle = nullptr;
    uint32_t ledAlarm1Channel = 0;
    uint32_t ledAlarm2Channel = 0;
    uint32_t ledRedChannel = 0;
    uint32_t ledGreenChannel = 0;

public:
    // APB1 for timers: 80MHz -> 1024 PWM steps and clock divison by 4 -> 19.5kHz PWM frequency
    static constexpr auto PwmSteps = 1024;
    static constexpr auto NumberOfResolutionBits = std::bit_width<size_t>(PwmSteps - 1);

    util::pwm_led::SingleLed<NumberOfResolutionBits> ledAlarm1{
        util::PwmOutput<NumberOfResolutionBits>{ledTimerHandle, ledAlarm1Channel}};

    util::pwm_led::SingleLed<NumberOfResolutionBits> ledAlarm2{
        util::PwmOutput<NumberOfResolutionBits>{ledTimerHandle, ledAlarm2Channel}};

    util::pwm_led::DualLed<NumberOfResolutionBits> ledRedGreen{
        util::PwmOutput<NumberOfResolutionBits>{ledTimerHandle, ledRedChannel},
        util::PwmOutput<NumberOfResolutionBits>{ledTimerHandle, ledGreenChannel}};

    void turnAllOn()
    {
        ledAlarm1.turnOn();
        ledAlarm2.turnOn();
        ledRedGreen.turnOn();
    }

    void turnAllOff()
    {
        ledAlarm1.turnOff();
        ledAlarm2.turnOff();
        ledRedGreen.turnOff();
    }
};