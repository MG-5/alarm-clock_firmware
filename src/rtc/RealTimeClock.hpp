#pragma once

#include "DS3231.hpp"
#include "state_machine/StateMachine.hpp"
#include "wrappers/Task.hpp"

class RealTimeClock : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    RealTimeClock(I2cAccessor &i2cAccessor, StateMachine &stateMachine)
        : TaskWithMemberFunctionBase("rtcTask", 256, osPriorityBelowNormal5), //
          i2cAccessor(i2cAccessor),                                           //
          stateMachine(stateMachine){};

protected:
    [[noreturn]] void taskMain(void *) override;

private:
    I2cAccessor &i2cAccessor;
    StateMachine &stateMachine;

    DS3231 rtcModule{i2cAccessor};

    void initRTC();
    bool isRtcOnline();
};