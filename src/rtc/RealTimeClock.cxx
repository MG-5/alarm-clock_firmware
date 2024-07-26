#include "RealTimeClock.hpp"
#include "helpers/freertos.hpp"
#include "sync.hpp"

void RealTimeClock::taskMain(void *)
{
    // start up RTC and load values
    while (true)
    {
        initRTC();

        if (!isRtcOnline())
        {
            vTaskDelay(toOsTicks(1.0_s));
            continue;
        }

        auto timeOptional = rtcModule.getTime();
        auto alarm1Optional = rtcModule.getAlarm1();
        auto alarm2Optional = rtcModule.getAlarm2();

        if (timeOptional && alarm1Optional && alarm2Optional)
        {
            clockTime = timeOptional.value();
            alarmTime1 = alarm1Optional.value();
            alarmTime2 = alarm2Optional.value();
            wasRtcOnlineOnceBool = true;

            // cap alarm minutes to factor 5
            alarmTime1.minute -= (alarmTime1.minute % 5);
            alarmTime2.minute -= (alarmTime2.minute % 5);
            writeAlarmTime1(alarmTime1);
            writeAlarmTime2(alarmTime2);

            break;
        }
    }

    syncEventGroup.setBits(sync::RtcHasRespondedOnce);

    // load current time every second
    auto lastWakeTime = xTaskGetTickCount();
    while (true)
    {
        auto timeValueOptional = rtcModule.getTime();

        if (timeValueOptional)
            clockTime = timeValueOptional.value();
        else
        {
            // ToDo: error handling (e.g. flag for state machine)

            // increment seconds as fallback
            clockTime.addSeconds(1);
        }
        vTaskDelayUntil(&lastWakeTime, toOsTicks(1.0_s));
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

//--------------------------------------------------------------------------------------------------
bool RealTimeClock::wasRtcOnlineOnce()
{
    return wasRtcOnlineOnceBool;
}

//--------------------------------------------------------------------------------------------------
Time RealTimeClock::getClockTime() const
{
    return clockTime;
}

//--------------------------------------------------------------------------------------------------
Time RealTimeClock::getAlarmTime1()
{
    auto timeOptional = rtcModule.getAlarm1();
    if (timeOptional)
        alarmTime1 = timeOptional.value();

    return alarmTime1;
}

//--------------------------------------------------------------------------------------------------
Time RealTimeClock::getAlarmTime2()
{
    auto timeOptional = rtcModule.getAlarm2();
    if (timeOptional)
        alarmTime2 = timeOptional.value();

    return alarmTime2;
}

//--------------------------------------------------------------------------------------------------
void RealTimeClock::writeAlarmTime1(Time &newAlarmTime)
{
    rtcModule.setAlarm1(newAlarmTime);
}

//--------------------------------------------------------------------------------------------------
void RealTimeClock::writeAlarmTime2(Time &newAlarmTime)
{
    rtcModule.setAlarm2(newAlarmTime);
}
