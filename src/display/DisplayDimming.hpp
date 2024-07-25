#pragma once

#include "core/SafeAssert.h"
#include "tim.h"

#include "util/MapValue.hpp"

class DisplayDimming
{
public:
    DisplayDimming(TIM_HandleTypeDef *multiplexingPwmTimer, uint32_t pwmTimChannel)
        : multiplexingPwmTimer(multiplexingPwmTimer), //
          pwmTimChannel(pwmTimChannel)
    {
        SafeAssert(multiplexingPwmTimer != nullptr);
    };

    static constexpr auto PwmMinimum = 55;
    static constexpr auto PwmMaximum = 249;

    void initPwm()
    {
        HAL_TIM_Base_Start_IT(multiplexingPwmTimer);
        HAL_TIM_OC_Start_IT(multiplexingPwmTimer, pwmTimChannel);
    }

    void stopPwm()
    {
        HAL_TIM_Base_Stop_IT(multiplexingPwmTimer);
        HAL_TIM_OC_Stop_IT(multiplexingPwmTimer, pwmTimChannel);
    }

    // 1% to 100%
    void setBrightness(uint8_t brightness)
    {
        if (brightness == 0 || brightness > 100)
            return;

        auto pwmValue = util::mapValue<size_t, size_t>(1, 100, PwmMinimum, PwmMaximum, brightness);
        __HAL_TIM_SetCompare(multiplexingPwmTimer, pwmTimChannel, pwmValue);
    }

private:
    TIM_HandleTypeDef *multiplexingPwmTimer;
    uint32_t pwmTimChannel;
};