#pragma once

#include "LED/LedStrip.hpp"
#include "LED/StatusLeds.hpp"
#include "buttons/Buttons.hpp"
#include "display/Display.hpp"
#include "rtc/RealTimeClock.hpp"

#include "util/gpio.hpp"
#include "wrappers/Task.hpp"

#include <climits>

class StateMachine : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    StateMachine(Display &display, StatusLeds &statusLeds, LedStrip &ledStrip, Buttons &buttons, RealTimeClock &rtc,
                 TimerCallbackFunction_t timeoutCallback)
        : TaskWithMemberFunctionBase("stateMachineTask", 512, osPriorityBelowNormal4), //
          display(display),                                                            //
          statusLeds(statusLeds),                                                      //
          ledStrip(ledStrip),                                                          //
          buttons(buttons),                                                            //
          rtc(rtc),                                                                    //
          timeoutCallback(timeoutCallback)
    {
        assignButtonCallbacks();
    }

    enum class DisplayState
    {
        Standby,
        Clock,
        ClockWithAlarmLeds,
        DisplayAlarm1,
        DisplayAlarm2,
        ChangeAlarm1Hour,
        ChangeAlarm1Minute,
        ChangeAlarm2Hour,
        ChangeAlarm2Minute,
        ChangeClockHour,
        ChangeClockMinute,
        DisplayAlarmStatus,
        LedBrightness,
        LedCCT,
        Test
    };

    enum class AlarmState
    {
        Off,
        Sunrise,
        Vibration,
        Snooze
    };

    enum class AlarmMode
    {
        Off,
        Alarm1,
        Alarm2,
        Both
    };

    void handleTimeoutTimer();

protected:
    void taskMain(void *) override;

private:
    Display &display;
    StatusLeds &statusLeds;
    LedStrip &ledStrip;
    Buttons &buttons;
    RealTimeClock &rtc;

    DisplayState displayState = DisplayState::Clock;
    DisplayState previousDisplayState = DisplayState::Standby;
    AlarmState alarmState = AlarmState::Off;
    AlarmMode alarmMode = AlarmMode::Both;
    bool blink = true;

    Time timeToModify;

    void displayLedInitialization();
    void waitForRtc();

    void showHourChanging();
    void showMinuteChanging();
    void showCurrentAlarmMode();
    void showCurrentBrightness();
    void showCurrentCCT();

    void updateDisplayState(DisplayState newState);
    void signalResult(bool success);
    void revokeDisplayDelay();
    void savePreviousState();
    void restorePreviousState();
    void goToDefaultState();

    void assignButtonCallbacks();

    void buttonLeftCallback(util::Button::Action action);
    void buttonRightCallback(util::Button::Action action);
    void buttonSnoozeCallback(util::Button::Action action);
    void buttonBrightnessPlusCallback(util::Button::Action action);
    void buttonBrightnessMinusCallback(util::Button::Action action);
    void buttonCCTPlusCallback(util::Button::Action action);
    void buttonCCTMinusCallback(util::Button::Action action);

    void incrementNumber();
    void decrementNumber();

    TimerCallbackFunction_t timeoutCallback = nullptr;

    // with enabled auto reload
    TimerHandle_t timeoutTimer{xTimerCreate("timeoutTimer", toOsTicks(4.0_s), pdTRUE, nullptr, timeoutCallback)};

    void setTimeoutAndStart(units::si::Time period)
    {
        setTimeoutTimerPeriod(period);
        resetTimeoutTimer();
    }

    void setTimeoutTimerPeriod(units::si::Time period)
    {
        xTimerChangePeriod(timeoutTimer, toOsTicks(period), 0);
    }

    void startTimeoutTimer()
    {
        if (xTimerIsTimerActive(timeoutTimer) == pdFALSE)
            xTimerStart(timeoutTimer, 0);
    }

    void stopTimeoutTimer()
    {
        xTimerStop(timeoutTimer, 0);
    }

    void resetTimeoutTimer()
    {
        xTimerReset(timeoutTimer, 0);
    }

    /// block task for specified time but can be unblocked by external event e.g. button press
    /// @return true if timeout is occurred
    bool delayUntilEventOrTimeout(units::si::Time blockTime, bool blockIndefinitely = false)
    {
        return notifyWait(ULONG_MAX, ULONG_MAX, (uint32_t *)0,
                          blockIndefinitely ? portMAX_DELAY : toOsTicks(blockTime)) == 0;
    }
};