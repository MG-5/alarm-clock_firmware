#pragma once

#include "State.hpp"

class ShowCurrentAlarmMode : public State
{
public:
    ShowCurrentAlarmMode(SystemComponents &systemComponents, StateEventCallback &stateEventCallback)
        : State(systemComponents, stateEventCallback) {};
    ~ShowCurrentAlarmMode() override = default;

    //-----------------------------------------------------------------
    void onEnter() override
    {
        xTimerReset(timeoutTimer, 0);
    }

    //-----------------------------------------------------------------
    void onExit() override
    {
        xTimerStop(timeoutTimer, 0);
    }

    //-----------------------------------------------------------------
    void draw() override
    {
        auto &display = systemComponents.display;
        auto &rtc = systemComponents.rtc;
        auto &statusLeds = systemComponents.statusLeds;

        display.clearGridDataArray();
        display.getGridDataArray()[2].segments = font.getGlyph('A');
        display.getGridDataArray()[2].enableDots = true;

        statusLeds.turnOffAlarmLeds();

        switch (rtc.getAlarmMode())
        {
        case RealTimeClock::AlarmMode::Off:
            display.getGridDataArray()[3].segments = font.getGlyph('O');
            display.getGridDataArray()[4].segments = font.getGlyph('f');
            display.getGridDataArray()[5].segments = font.getGlyph('f');
            break;

        case RealTimeClock::AlarmMode::Alarm1:
            display.getGridDataArray()[4].segments = font.getGlyph('1');
            statusLeds.ledAlarm1.turnOn();
            break;

        case RealTimeClock::AlarmMode::Alarm2:
            display.getGridDataArray()[4].segments = font.getGlyph('2');
            statusLeds.ledAlarm2.turnOn();
            break;

        case RealTimeClock::AlarmMode::Both:
            display.getGridDataArray()[3].segments = font.getGlyph('1');
            display.getGridDataArray()[4].segments = font.getGlyph('+');
            display.getGridDataArray()[5].segments = font.getGlyph('2');
            statusLeds.ledAlarm1.turnOn();
            statusLeds.ledAlarm2.turnOn();
            break;
        }

        setUpdateDelay(1.0_s);
    }

    //-----------------------------------------------------------------
    std::optional<StateId> onButtonEvent(Buttons::ButtonId buttonId, util::Button::Action action) override
    {
        auto &rtc = systemComponents.rtc;

        if (buttonId == Buttons::ButtonId::Right && action == util::Button::Action::ShortPress)
        {
            switch (rtc.getAlarmMode())
            {
            case RealTimeClock::AlarmMode::Off:
                rtc.setAlarmMode(RealTimeClock::AlarmMode::Alarm1);
                break;

            case RealTimeClock::AlarmMode::Alarm1:
                rtc.setAlarmMode(RealTimeClock::AlarmMode::Alarm2);
                break;

            case RealTimeClock::AlarmMode::Alarm2:
                rtc.setAlarmMode(RealTimeClock::AlarmMode::Both);
                break;

            case RealTimeClock::AlarmMode::Both:
                rtc.setAlarmMode(RealTimeClock::AlarmMode::Off);
                break;
            }

            revokeRedrawAndTimeout();
        }
        return std::nullopt;
    }

    // -----------------------------------------------------------------
    void revokeRedrawAndTimeout()
    {
        requestRedraw();
        xTimerReset(timeoutTimer, 0);
    }

    //-----------------------------------------------------------------
    static void timeoutCallback(TimerHandle_t xTimer)
    {
        auto showCurrentAlarmMode = static_cast<ShowCurrentAlarmMode *>(pvTimerGetTimerID(xTimer));

        showCurrentAlarmMode->requestStateChange(StateId::Clock);
    }

private:
    TimerHandle_t timeoutTimer{xTimerCreate("showCurrentAlarmModeTimeout", toOsTicks(3.0_s), pdFALSE, this,
                                            &ShowCurrentAlarmMode::timeoutCallback)};
};