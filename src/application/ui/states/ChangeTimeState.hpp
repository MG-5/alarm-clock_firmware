#pragma once

#include "State.hpp"

class ChangeTimeState : public State
{
public:
    enum class TimeToModify
    {
        Alarm1,
        Alarm2,
        Clock
    };

    ChangeTimeState(SystemComponents &systemComponents, TimeToModify timeToModify,
                    StateEventCallback &stateEventCallback)
        : State(systemComponents, stateEventCallback), //
          timeToModify(timeToModify) {};

    ~ChangeTimeState() override = default;

    //-----------------------------------------------------------------
    void onEnter() override
    {
        blink = true;
        timeType = TimeType::Hour;
        isTimeModified = false;
        saveTime = false;

        targetTime = (timeToModify == TimeToModify::Alarm1)   ? systemComponents.rtc.getAlarmTime1()
                     : (timeToModify == TimeToModify::Alarm2) ? systemComponents.rtc.getAlarmTime2()
                                                              : systemComponents.rtc.getClockTime();
    }

    //-----------------------------------------------------------------
    void onExit() override
    {
        xTimerStop(timeoutTimer, 0);

        if (!(isTimeModified && saveTime))
            return;

        if (timeToModify == TimeToModify::Alarm1)
        {
            signalResult(systemComponents.rtc.writeAlarmTime1(targetTime));
            systemComponents.rtc.setAlarmMode(RealTimeClock::AlarmMode::Alarm1);
        }
        else if (timeToModify == TimeToModify::Alarm2)
        {
            signalResult(systemComponents.rtc.writeAlarmTime2(targetTime));
            systemComponents.rtc.setAlarmMode(RealTimeClock::AlarmMode::Alarm2);
        }
        else if (timeToModify == TimeToModify::Clock)
            signalResult(systemComponents.rtc.writeClockTime(targetTime));
    }

    //-----------------------------------------------------------------
    void draw() override
    {
        systemComponents.display.clearGridDataArray();

        if (timeType == TimeType::Hour)
            renderHourChanging();
        else
            renderMinuteChanging();

        blink = !blink;

        setUpdateDelay(500.0_ms);
    }

    //-----------------------------------------------------------------
    std::optional<StateId> onButtonEvent(Buttons::ButtonId buttonId, util::Button::Action action) override
    {
        switch (action)
        {
        case util::Button::Action::ShortPress:
        {
            requestRedraw();

            switch (buttonId)
            {
            case Buttons::ButtonId::Left:
            {
                blink = true;
                switch (timeType)
                {
                case TimeType::Hour:
                    timeType = TimeType::Minute;
                    break;

                case TimeType::Minute:
                    saveTime = true;
                    return StateId::Clock;
                    break;
                }
            }
            break;

            case Buttons::ButtonId::Right:
            case Buttons::ButtonId::BrightnessPlus:
            case Buttons::ButtonId::CCTPlus:
                incrementNumber();
                break;

            case Buttons::ButtonId::BrightnessMinus:
            case Buttons::ButtonId::CCTMinus:
                decrementNumber();
                break;

            default:
                break;
            }
        }
        break;

        case util::Button::Action::LongPress:
        {
            if (buttonId == Buttons::ButtonId::Right || buttonId == Buttons::ButtonId::BrightnessPlus ||
                buttonId == Buttons::ButtonId::CCTPlus)
            {
                xTimerReset(timeoutTimer, 0);
                isIncrementing = true;
                incrementNumber();
            }
            else if (buttonId == Buttons::ButtonId::BrightnessMinus || buttonId == Buttons::ButtonId::CCTMinus)
            {
                xTimerReset(timeoutTimer, 0);
                isIncrementing = false;
                decrementNumber();
            }
        }
        break;

        case util::Button::Action::StopLongPress:
            xTimerStop(timeoutTimer, 0);
            break;

        default:
            break;
        }

        return std::nullopt;
    }

    //-----------------------------------------------------------------
    static void timeoutCallback(TimerHandle_t xTimer)
    {
        auto changeTimeState = static_cast<ChangeTimeState *>(pvTimerGetTimerID(xTimer));

        if (changeTimeState->isIncrementing)
            changeTimeState->incrementNumber();

        else
            changeTimeState->decrementNumber();

        changeTimeState->blink = false;
        changeTimeState->requestRedraw();
    }

private:
    enum class TimeType
    {
        Hour,
        Minute
    } timeType = TimeType::Hour;

    TimeToModify timeToModify;

    Time targetTime;

    bool isTimeModified = false;
    bool saveTime = false;

    bool isIncrementing = true;
    bool blink = true;

    TimerHandle_t timeoutTimer{
        xTimerCreate("changeTimeTimeout", toOsTicks(250.0_ms), pdTRUE, this, &ChangeTimeState::timeoutCallback)};

    void signalResult(bool success)
    {
        success ? systemComponents.statusLeds.signalSuccess() : systemComponents.statusLeds.signalError();
    }

    void incrementNumber()
    {
        blink = false;
        isTimeModified = true;
        if (timeType == TimeType::Hour)
            targetTime.addHours(1);

        else
            targetTime.addMinutes(timeToModify == TimeToModify::Clock ? 1 : 5);

        requestRedraw();
    }

    void decrementNumber()
    {
        blink = false;
        isTimeModified = true;
        if (timeType == TimeType::Hour)
            targetTime.subHours(1);

        else
            targetTime.subMinutes(timeToModify == TimeToModify::Clock ? 1 : 5);

        requestRedraw();
    }

    void renderHourChanging()
    {
        auto &display = systemComponents.display;

        display.setClock(targetTime);
        display.renderClock(true);

        display.getGridDataArray()[1].enableUpperBar = display.getGridDataArray()[2].enableUpperBar = blink;
        if (blink)
            // show no digits
            display.getGridDataArray()[1].segments = display.getGridDataArray()[2].segments = 0;
    }

    void renderMinuteChanging()
    {
        auto &display = systemComponents.display;

        display.setClock(targetTime);
        display.renderClock(true);

        display.getGridDataArray()[3].enableUpperBar = display.getGridDataArray()[4].enableUpperBar = blink;
        if (blink)
            // show no digits
            display.getGridDataArray()[3].segments = display.getGridDataArray()[4].segments = 0;
    }
};