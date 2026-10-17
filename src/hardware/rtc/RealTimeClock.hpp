#pragma once

#include "DS3231.hpp"
#include "wrappers/Task.hpp"

#include "application/wakeup/WakeupController.hpp"
#include "hardware/buttons/Buttons.hpp"

class RealTimeClock : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    RealTimeClock(I2cAccessor &i2cAccessor, WakeupController &wakeupController)
        : TaskWithMemberFunctionBase("rtcTask", 256, osPriorityBelowNormal5), //
          i2cAccessor(i2cAccessor), wakeupController(wakeupController)
    {
    }

    enum class AlarmMode
    {
        Off,
        Alarm1,
        Alarm2,
        Both
    };

    bool isRtcOnline();
    bool wasRtcOnlineOnce();

    Time getClockTime() const;
    Time getAlarmTime1();
    Time getAlarmTime2();

    bool writeAlarmTime1(Time &newAlarmTime);
    bool writeAlarmTime2(Time &newAlarmTime);
    bool writeClockTime(Time &newClockTime);

    void setAlarmMode(AlarmMode newMode)
    {
        alarmMode = newMode;
    }

    AlarmMode getAlarmMode() const
    {
        return alarmMode;
    }

    void handleButtonEvents(Buttons::ButtonId buttonId, util::Button::Action action);

protected:
    [[noreturn]] void taskMain(void *) override;

private:
    I2cAccessor &i2cAccessor;
    DS3231 rtcModule{i2cAccessor};

    WakeupController &wakeupController;

    bool wasRtcOnlineOnceBool = false;

    Time clockTime;
    Time alarmTime1;
    Time alarmTime2;

    AlarmMode alarmMode = AlarmMode::Both;

    void initRTC();
    void setupRtcAndAlarms();
    void fetchClockTime();

    void determineAlarmTriggerState();
};