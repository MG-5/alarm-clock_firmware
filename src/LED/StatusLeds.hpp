#pragma once

#include "tim.h"
#include "util/PwmLed.hpp"
#include "wrappers/Task.hpp"
#include <bit>

class StatusLeds : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    StatusLeds(TIM_HandleTypeDef *ledTimerHandle, uint32_t ledAlarm1Channel,
               uint32_t ledAlarm2Channel, uint32_t ledRedChannel, uint32_t ledGreenChannel,
               TimerCallbackFunction_t timeoutCallback)
        : TaskWithMemberFunctionBase("statusLedTask", 128, osPriorityLow2),
          ledTimerHandle(ledTimerHandle),     //
          ledAlarm1Channel(ledAlarm1Channel), //
          ledAlarm2Channel(ledAlarm2Channel), //
          ledRedChannel(ledRedChannel),       //
          ledGreenChannel(ledGreenChannel),   //
          timeoutCallback(timeoutCallback)
    {
        SafeAssert(this->ledTimerHandle != nullptr);
    }

    void handleTimeoutTimer()
    {
        ledRedGreen.turnOff();
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
            vTaskDelayUntil(&lastWakeTime, toOsTicks(100.0_Hz));
        }
    }

private:
    TIM_HandleTypeDef *ledTimerHandle = nullptr;
    uint32_t ledAlarm1Channel = 0;
    uint32_t ledAlarm2Channel = 0;
    uint32_t ledRedChannel = 0;
    uint32_t ledGreenChannel = 0;

    TimerCallbackFunction_t timeoutCallback = nullptr;
    TimerHandle_t timeoutTimer{
        xTimerCreate("timeoutTimer", toOsTicks(2.0_s), pdFALSE, nullptr, timeoutCallback)};

public:
    // APB1 for timers: 80MHz -> 1024 PWM steps and clock divison by 4 -> 19.5kHz PWM frequency
    static constexpr auto PwmSteps = 1024;
    static constexpr auto NumberOfResolutionBits = std::bit_width<size_t>(PwmSteps - 1);

    using SingleLed = util::pwm_led::SingleLed<NumberOfResolutionBits>;

    SingleLed ledAlarm1{util::PwmOutput<NumberOfResolutionBits>{ledTimerHandle, ledAlarm1Channel}};
    SingleLed ledAlarm2{util::PwmOutput<NumberOfResolutionBits>{ledTimerHandle, ledAlarm2Channel}};

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

    void signalSuccess()
    {
        ledRedGreen.setColor(util::pwm_led::DualLedColor::Green);
        xTimerChangePeriod(timeoutTimer, toOsTicks(1.0_s), 0);
        xTimerReset(timeoutTimer, 0);
    }

    void signalError()
    {
        ledRedGreen.setColor(util::pwm_led::DualLedColor::Red);
        xTimerChangePeriod(timeoutTimer, toOsTicks(5.0_s), 0);
        xTimerReset(timeoutTimer, 0);
    }
};