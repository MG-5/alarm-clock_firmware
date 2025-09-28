#pragma once

#include "tim.h"
#include "util/led/PwmLed.hpp"
#include "wrappers/Task.hpp"

class StatusLeds : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    StatusLeds(TIM_HandleTypeDef *ledTimerHandle, const uint32_t &ledAlarm1Channel, const uint32_t &ledAlarm2Channel,
               const uint32_t &ledRedChannel, const uint32_t &ledGreenChannel, TimerCallbackFunction_t timeoutCallback)
        : TaskWithMemberFunctionBase("statusLedTask", 128, osPriorityLow2), ledTimerHandle(ledTimerHandle), //
          ledAlarm1Channel(ledAlarm1Channel),                                                               //
          ledAlarm2Channel(ledAlarm2Channel),                                                               //
          ledRedChannel(ledRedChannel),                                                                     //
          ledGreenChannel(ledGreenChannel),                                                                 //
          timeoutCallback(timeoutCallback)
    {
        configASSERT(this->ledTimerHandle != nullptr);
    }

    void handleTimeoutTimer()
    {
        ledRedGreen.turnOff();
    }

    void setGlobalBrightness(uint8_t newBrightness)
    {
        globalBrightness = newBrightness;
        updateBrightness();
    }

    uint8_t getGlobalBrightness() const
    {
        return globalBrightness;
    }

protected:
    [[noreturn]] void taskMain(void *)
    {
        auto lastWakeTime = xTaskGetTickCount();
        updateBrightness();
        turnAllOff();

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
    const uint32_t &ledAlarm1Channel;
    const uint32_t &ledAlarm2Channel;
    const uint32_t &ledRedChannel;
    const uint32_t &ledGreenChannel;

    TimerCallbackFunction_t timeoutCallback = nullptr;
    TimerHandle_t timeoutTimer{xTimerCreate("timeoutTimer", toOsTicks(2.0_s), pdFALSE, nullptr, timeoutCallback)};

public:
    // APB1 for timers: 80MHz -> 1024 PWM steps and clock divison by 4 -> 19.5kHz PWM frequency
    static constexpr auto PwmSteps = 1024;
    static constexpr auto ResolutionBits = std::bit_width<size_t>(PwmSteps - 1);
    using GammaCorrection_t = util::led::pwm::GammaCorrection<ResolutionBits>;
    static constexpr GammaCorrection_t GammaCorrection{};

    uint8_t globalBrightness = 25;

    using SingleLed = util::led::pwm::SingleLed<ResolutionBits, GammaCorrection_t>;
    using DualLed = util::led::pwm::DualLed<ResolutionBits, GammaCorrection_t>;

    SingleLed ledAlarm1{util::PwmOutput<ResolutionBits>{ledTimerHandle, ledAlarm1Channel}, GammaCorrection};
    SingleLed ledAlarm2{util::PwmOutput<ResolutionBits>{ledTimerHandle, ledAlarm2Channel}, GammaCorrection};

    DualLed ledRedGreen{util::PwmOutput<ResolutionBits>{ledTimerHandle, ledRedChannel},
                        util::PwmOutput<ResolutionBits>{ledTimerHandle, ledGreenChannel}, GammaCorrection};

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
        ledRedGreen.setColor(util::led::pwm::DualLedColor::Green);
        xTimerChangePeriod(timeoutTimer, toOsTicks(1.0_s), 0);
        xTimerReset(timeoutTimer, 0);
    }

    void signalError()
    {
        ledRedGreen.setColor(util::led::pwm::DualLedColor::Red);
        xTimerChangePeriod(timeoutTimer, toOsTicks(5.0_s), 0);
        xTimerReset(timeoutTimer, 0);
    }

    void updateBrightness()
    {
        ledAlarm1.setLightLevel(PwmSteps * globalBrightness / 100);
        ledAlarm2.setLightLevel(PwmSteps * globalBrightness / 100);
        ledRedGreen.setBrightness(globalBrightness);
    }
};