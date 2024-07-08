#pragma once

#include "main.h"

#include "helpers/freertos.hpp"
#include "util/Button.hpp"
#include "wrappers/Task.hpp"

/// all buttons are handled here, incl. debouncing, long press detection etc
class Buttons : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    Buttons() : TaskWithMemberFunctionBase("buttonsPollingTask", 128, osPriorityBelowNormal3){};

    util::Button left{{ButtonLeft_GPIO_Port, ButtonLeft_Pin}};
    util::Button right{{ButtonRight_GPIO_Port, ButtonRight_Pin}};
    util::Button snooze{{ButtonSnooze_GPIO_Port, ButtonSnooze_Pin}};

    util::Button brightnessPlus{{ButtonBrightnessPlus_GPIO_Port, ButtonBrightnessPlus_Pin}};
    util::Button brightnessMinus{{ButtonBrightnessMinus_GPIO_Port, ButtonBrightnessMinus_Pin}};
    util::Button cctPlus{{ButtonCCTPlus_GPIO_Port, ButtonCCTPlus_Pin}};
    util::Button cctMinus{{ButtonCCTMinus_GPIO_Port, ButtonCCTMinus_Pin}};

protected:
    [[noreturn]] void taskMain(void *) override;
};