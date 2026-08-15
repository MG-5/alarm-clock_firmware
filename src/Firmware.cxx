#include "FreeRTOS.h"
#include "main.h"
#include "task.h"

#include "Firmware.hpp"
#include "wrappers/Task.hpp"

#include <memory>

extern "C" void StartDefaultTask(void *) // NOLINT
{
    static auto app = std::make_unique<Firmware>();
    app->run();

    configASSERT(false); // this line should be never reached
}

//--------------------------------------------------------------------------------------------------
Firmware::Firmware()
{
    // Delegated Singleton, see getApplicationInstance() for further explanations
    configASSERT(instance == nullptr);
    instance = this;

    registerCallbacks();
}

//--------------------------------------------------------------------------------------------------
[[noreturn]] void Firmware::run()
{
    util::wrappers::Task::applicationIsReadyStartAllTasks();
    while (true)
    {
        vTaskDelay(portMAX_DELAY);
    }
}

//--------------------------------------------------------------------------------------------------
Firmware &Firmware::getApplicationInstance()
{
    // Not constructing Application in this singleton, to avoid bugs where something tries to
    // access this function, while application constructs which will cause infinite recursion
    return *instance;
}

//--------------------------------------------------------------------------------------------------
void Firmware::registerCallbacks()
{
    HAL_StatusTypeDef result = HAL_OK;

    result = HAL_I2C_RegisterCallback(RtcBus, HAL_I2C_MASTER_TX_COMPLETE_CB_ID, [](I2C_HandleTypeDef *)
                                      { getApplicationInstance().i2cBusAccessor.signalTransferCompleteFromIsr(); });
    configASSERT(result == HAL_OK);

    result = HAL_I2C_RegisterCallback(RtcBus, HAL_I2C_MASTER_RX_COMPLETE_CB_ID, [](I2C_HandleTypeDef *)
                                      { getApplicationInstance().i2cBusAccessor.signalTransferCompleteFromIsr(); });
    configASSERT(result == HAL_OK);

    result = HAL_I2C_RegisterCallback(RtcBus, HAL_I2C_ERROR_CB_ID, [](I2C_HandleTypeDef *)
                                      { getApplicationInstance().i2cBusAccessor.signalErrorFromIsr(); });
    configASSERT(result == HAL_OK);
}

//--------------------------------------------------------------------------------------------------
void Firmware::multiplexingTimerUpdate()
{
    getApplicationInstance().display.multiplexingInterrupt();
}

//--------------------------------------------------------------------------------------------------
void Firmware::pwmTimerCompare()
{
    getApplicationInstance().display.pwmTimerInterrupt();
}

//--------------------------------------------------------------------------------------------------
// skip HAL`s interupt routine to get more performance
extern "C" void TIM1_UP_TIM16_IRQHandler(void)
{
    if (__HAL_TIM_GET_FLAG(Firmware::MultiplexingPwmTimer, TIM_FLAG_UPDATE) == SET)
    {
        __HAL_TIM_CLEAR_IT(Firmware::MultiplexingPwmTimer, TIM_IT_UPDATE);
        Firmware::multiplexingTimerUpdate();
    }
    else if (__HAL_TIM_GET_FLAG(Firmware::MultiplexingPwmTimer, TIM_FLAG_CC1) == SET)
    {
        __HAL_TIM_CLEAR_IT(Firmware::MultiplexingPwmTimer, TIM_IT_CC1);
        Firmware::pwmTimerCompare();
    }
}