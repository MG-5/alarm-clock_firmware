#pragma once

#include "wrappers/Task.hpp"

#include "State.hpp"

/// State representing the clock display
class ClockState : public State
{
public:
    ClockState(SystemComponents &resources, StateEventCallback &stateEventCallback)
        : State(resources, stateEventCallback)
    {
        configASSERT(timeoutTimer != nullptr);
    }

    ~ClockState() override = default;

    //-----------------------------------------------------------------
    void onEnter() override
    {
        showAlarmLeds = true;
        shoudGoToStandby = false;
        setTimeoutPeriod(ActiveStateTimeout);
        resetTimeoutTimer();
    }

    //-----------------------------------------------------------------
    void onExit() override
    {
        stopTimeoutTimer();
        systemComponents.display.setBrightness(100);
    }

    //-----------------------------------------------------------------
    void draw() override
    {
        auto &display = systemComponents.display;
        auto &rtc = systemComponents.rtc;
        auto &statusLeds = systemComponents.statusLeds;

        if (shoudGoToStandby)
        {
            requestStateChange(StateId::Standby);
            return;
        }

        display.setClock(rtc.getClockTime());
        display.renderClock();

        if (showAlarmLeds)
        {
            statusLeds.ledAlarm1.setState(rtc.getAlarmMode() == RealTimeClock::AlarmMode::Alarm1 ||
                                          rtc.getAlarmMode() == RealTimeClock::AlarmMode::Both);
            statusLeds.ledAlarm2.setState(rtc.getAlarmMode() == RealTimeClock::AlarmMode::Alarm2 ||
                                          rtc.getAlarmMode() == RealTimeClock::AlarmMode::Both);
            display.setBrightness(100);
        }
        else
        {
            statusLeds.ledAlarm1.turnOff();
            statusLeds.ledAlarm2.turnOff();
            display.setBrightness(25);
        }

        setUpdateDelay(100.0_ms);
    }

    //-----------------------------------------------------------------
    std::optional<StateId> onButtonEvent(Buttons::ButtonId buttonId, util::Button::Action action) override
    {
        switch (buttonId)
        {
        case Buttons::ButtonId::Left:
        {
            if (action == util::Button::Action::ShortPress)
                return StateId::AlarmTimes;

            else if (action == util::Button::Action::SuperLongPress)
                return StateId::Standby;
        }
        break;

        case Buttons::ButtonId::Right:
        {
            if (action == util::Button::Action::ShortPress)
                return StateId::CurrentAlarm;
        }
        break;

        case Buttons::ButtonId::Snooze:
        {
            if (action == util::Button::Action::ShortPress)
            {
                showAlarmLeds = true;
                setTimeoutPeriod(ActiveStateTimeout);
                resetTimeoutTimer();

                requestRedraw();
            }
        }

        default:
            break;
        }

        return std::nullopt;
    }

    //-----------------------------------------------------------------
    static void timeoutCallback(TimerHandle_t xTimer)
    {
        (void)xTimer;
        auto clockState = static_cast<ClockState *>(pvTimerGetTimerID(xTimer));

        if (clockState->showAlarmLeds)
        {
            clockState->showAlarmLeds = false;
            clockState->evaluateStandbyCondition();
        }
        else
        {
            // if we are in normal clock display mode (w/o alarm LEDs), go to standby
            clockState->shoudGoToStandby = true;
        }
    }

    //-----------------------------------------------------------------
    void evaluateStandbyCondition()
    {
        // ToDo: wake up after 7:00

        if (systemComponents.ledStrip.getState() == LedStrip::State::Off ||
            systemComponents.ledStrip.getState() == LedStrip::State::FadingOff)
        {
            // go to standby between 23:00 and 7:00
            if (systemComponents.rtc.getClockTime().hour >= 23 || systemComponents.rtc.getClockTime().hour < 7)
            {
                if (!isTimerActive())
                {
                    setTimeoutPeriod(10.0_s);
                    resetTimeoutTimer();
                }
            }
        }
    }

private:
    bool showAlarmLeds = true;
    bool shoudGoToStandby = false;

    static constexpr auto ActiveStateTimeout = 5.0_s;

    TimerHandle_t timeoutTimer{
        xTimerCreate("clockStateTimeout", toOsTicks(10.0_s), pdFALSE, this, &ClockState::timeoutCallback)};

    //-----------------------------------------------------------------
    void setTimeoutPeriod(units::si::Time period)
    {
        xTimerChangePeriod(timeoutTimer, toOsTicks(period), 0);
    }

    //-----------------------------------------------------------------
    void resetTimeoutTimer()
    {
        xTimerReset(timeoutTimer, 0);
    }

    //-----------------------------------------------------------------
    void stopTimeoutTimer()
    {
        xTimerStop(timeoutTimer, 0);
    }

    //-----------------------------------------------------------------
    bool isTimerActive()
    {
        return xTimerIsTimerActive(timeoutTimer);
    }
};