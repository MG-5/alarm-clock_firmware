#pragma once

#include "DS3231.hpp"
#include "wrappers/Task.hpp"

class RealTimeClock : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    RealTimeClock(I2cAccessor &i2cAccessor)
        : TaskWithMemberFunctionBase("rtcTask", 256, osPriorityBelowNormal5), //
          i2cAccessor(i2cAccessor) {};

    bool isRtcOnline();
    bool wasRtcOnlineOnce();

    Time getClockTime() const;
    Time getAlarmTime1();
    Time getAlarmTime2();

    bool writeAlarmTime1(Time &newAlarmTime);
    bool writeAlarmTime2(Time &newAlarmTime);
    bool writeClockTime(Time &newClockTime);

protected:
    [[noreturn]] void taskMain(void *) override;

private:
    I2cAccessor &i2cAccessor;
    DS3231 rtcModule{i2cAccessor};

    bool wasRtcOnlineOnceBool = false;

    Time clockTime;
    Time alarmTime1;
    Time alarmTime2;

    void initRTC();
};