#include "RealTimeClock.hpp"
#include "helpers/freertos.hpp"
#include "sync.hpp"

void RealTimeClock::taskMain(void *)
{
    // try to connect to RTC
    while (true)
    {
        initRTC();

        if (isRtcOnline())
            break;

        vTaskDelay(toOsTicks(1.0_s));
    }

    syncEventGroup.setBits(sync::WaitForRtc);

    while (true)
    {
        auto timeValueOptional = rtcModule.getTime();
        if (timeValueOptional)
            stateMachine.updateClock(timeValueOptional.value());

        vTaskDelay(toOsTicks(1.0_s));
    }
}

//--------------------------------------------------------------------------------------------------
void RealTimeClock::initRTC()
{
    rtcModule.enable32KHz(false);
    rtcModule.setInterruptOutput(true);
    rtcModule.clearAlarm1Flag();
    rtcModule.clearAlarm2Flag();
}

//--------------------------------------------------------------------------------------------------
bool RealTimeClock::isRtcOnline()
{
    return !rtcModule.isCommunicationFailed();
}
