#pragma once

#include "util/led/BinaryLed.hpp"
#include "wrappers/Task.hpp"

class VibrationController : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    VibrationController() : TaskWithMemberFunctionBase("vibrationControllerTask", 128, osPriorityLow2) {};

    void startVibration(units::si::Frequency frequency)
    {
        vibrationMotor.setBlinking(frequency);
    }

    void stopVibration()
    {
        vibrationMotor.turnOff();
    }

protected:
    [[noreturn]] void taskMain(void *) override
    {
        auto lastWakeTime = xTaskGetTickCount();
        while (true)
        {
            vibrationMotor.updateState(xTaskGetTickCount());
            vTaskDelayUntil(&lastWakeTime, toOsTicks(10.0_Hz));
        }
    }

private:
    // declare a binary led object to control the vibration motor with a frequency
    util::led::binary::SingleLed vibrationMotor{{VibrationCushion_GPIO_Port, VibrationCushion_Pin}};
};