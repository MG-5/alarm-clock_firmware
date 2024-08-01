#include "FreeRTOS.h"
#include "main.h"
#include "task.h"

#include "Application.hpp"
#include "core/SafeAssert.h"
#include "wrappers/Task.hpp"

#include <memory>

extern "C" void StartDefaultTask(void *) // NOLINT
{
    static auto app = std::make_unique<Application>();
    app->run();

    SafeAssert(false); // this line should be never reached
}

//--------------------------------------------------------------------------------------------------
Application::Application()
{
    // Delegated Singleton, see getApplicationInstance() for further explanations
    SafeAssert(instance == nullptr);
    instance = this;

    registerCallbacks();
}

//--------------------------------------------------------------------------------------------------
[[noreturn]] void Application::run()
{
    util::wrappers::Task::applicationIsReadyStartAllTasks();
    while (true)
    {
        vTaskDelay(portMAX_DELAY);
    }
}

//--------------------------------------------------------------------------------------------------
Application &Application::getApplicationInstance()
{
    // Not constructing Application in this singleton, to avoid bugs where something tries to
    // access this function, while application constructs which will cause infinite recursion
    return *instance;
}

//--------------------------------------------------------------------------------------------------
void Application::registerCallbacks()
{
    HAL_StatusTypeDef result = HAL_OK;

    result = HAL_I2C_RegisterCallback(
        RtcBus, HAL_I2C_MASTER_TX_COMPLETE_CB_ID, [](I2C_HandleTypeDef *)
        { getApplicationInstance().i2cBusAccessor.signalTransferCompleteFromIsr(); });
    SafeAssert(result == HAL_OK);

    result = HAL_I2C_RegisterCallback(
        RtcBus, HAL_I2C_MASTER_RX_COMPLETE_CB_ID, [](I2C_HandleTypeDef *)
        { getApplicationInstance().i2cBusAccessor.signalTransferCompleteFromIsr(); });
    SafeAssert(result == HAL_OK);

    result =
        HAL_I2C_RegisterCallback(RtcBus, HAL_I2C_ERROR_CB_ID, [](I2C_HandleTypeDef *)
                                 { getApplicationInstance().i2cBusAccessor.signalErrorFromIsr(); });
    SafeAssert(result == HAL_OK);
}

//--------------------------------------------------------------------------------------------------
void Application::multiplexingTimerUpdate()
{
    getApplicationInstance().display.multiplexingInterrupt();
}

//--------------------------------------------------------------------------------------------------
void Application::pwmTimerCompare()
{
    getApplicationInstance().display.pwmTimerInterrupt();
}

//--------------------------------------------------------------------------------------------------
// skip HAL`s interupt routine to get more performance

extern "C" void TIM1_UP_TIM16_IRQHandler(void)
{
    __HAL_TIM_CLEAR_IT(Application::MultiplexingPwmTimer, TIM_IT_UPDATE);
    Application::multiplexingTimerUpdate();
}

//--------------------------------------------------------------------------------------------------
extern "C" void TIM1_CC_IRQHandler(void)
{
    if (__HAL_TIM_GET_FLAG(Application::MultiplexingPwmTimer, TIM_FLAG_CC1) == SET)
    {
        if (__HAL_TIM_GET_IT_SOURCE(Application::MultiplexingPwmTimer, TIM_IT_CC1) == SET)
        {
            __HAL_TIM_CLEAR_IT(Application::MultiplexingPwmTimer, TIM_IT_CC1);
            Application::pwmTimerCompare();
        }
    }
}

//--------------------------------------------------------------------------------------------------
void Application::statusLedsTimeoutCallback(TimerHandle_t timer)
{
    getApplicationInstance().statusLeds.handleTimeoutTimer();
}

//--------------------------------------------------------------------------------------------------
void Application::stateMachineTimeoutCallback(TimerHandle_t timer)
{
    getApplicationInstance().stateMachine.handleTimeoutTimer();
}