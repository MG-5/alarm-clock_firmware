#include "StateMachine.hpp"

void StateMachine::handleTimeoutTimer()
{
    switch (displayState)
    {
    case DisplayState::ClockWithAlarmLeds:
        stopTimeoutTimer();
        updateDisplayState(DisplayState::Clock);
        break;

    case DisplayState::ChangeAlarm1Hour:
    case DisplayState::ChangeAlarm2Hour:
    case DisplayState::ChangeClockHour:
        blink = false;
        timeToModify.addHours(1);
        revokeDisplayDelay();
        break;

    case DisplayState::ChangeAlarm1Minute:
    case DisplayState::ChangeAlarm2Minute:
        blink = false;
        timeToModify.addMinutes(5);
        revokeDisplayDelay();
        break;

    case DisplayState::ChangeClockMinute:
        blink = false;
        timeToModify.addMinutes(1);
        revokeDisplayDelay();
        break;

    case DisplayState::LedBrightness:
    case DisplayState::LedCCT:
        restorePreviousState();
        break;

    default: // as fallback
        stopTimeoutTimer();
        break;
    }
}

//-----------------------------------------------------------------
void StateMachine::buttonLeftCallback(util::Button::Action action)
{
    switch (action)
    {
    case util::Button::Action::ShortPress:
    {
        if (alarmState != AlarmState::Off)
            return;

        switch (displayState)
        {
        case DisplayState::Clock:
        case DisplayState::ClockWithAlarmLeds:
            timeToModify = rtc.getAlarmTime1();
            updateDisplayState(DisplayState::DisplayAlarm1);
            break;

        case DisplayState::DisplayAlarm1:
            timeToModify = rtc.getAlarmTime2();
            updateDisplayState(DisplayState::DisplayAlarm2);
            break;

        case DisplayState::DisplayAlarm2:
            goToDefaultState();
            break;

        case DisplayState::ChangeAlarm1Hour:
            updateDisplayState(DisplayState::ChangeAlarm1Minute);
            break;

        case DisplayState::ChangeAlarm1Minute:
            signalResult(rtc.writeAlarmTime1(timeToModify));
            alarmMode = AlarmMode::Alarm1;
            updateDisplayState(DisplayState::DisplayAlarm1);
            break;

        case DisplayState::ChangeAlarm2Hour:
            updateDisplayState(DisplayState::ChangeAlarm2Minute);
            break;

        case DisplayState::ChangeAlarm2Minute:
            signalResult(rtc.writeAlarmTime2(timeToModify));
            alarmMode = AlarmMode::Alarm2;
            updateDisplayState(DisplayState::DisplayAlarm2);
            break;

        case DisplayState::ChangeClockHour:
            updateDisplayState(DisplayState::ChangeClockMinute);
            break;

        case DisplayState::ChangeClockMinute:
            signalResult(rtc.writeClockTime(timeToModify));
            goToDefaultState();
            break;

        case DisplayState::Standby:
            goToDefaultState();
            break;

        default:
            break;
        }

        blink = true;
    }
    break;

    case util::Button::Action::LongPress:
    {
        if (alarmState != AlarmState::Off)
            return;

        switch (displayState)
        {
        case DisplayState::DisplayAlarm1:
            blink = true;
            updateDisplayState(DisplayState::ChangeAlarm1Hour);
            break;

        case DisplayState::DisplayAlarm2:
            blink = true;
            updateDisplayState(DisplayState::ChangeAlarm2Hour);
            break;

        default:
            break;
        }
        break;
    }
    case util::Button::Action::SuperLongPress:
    {
        if (alarmState != AlarmState::Off)
        {
            alarmState = AlarmState::Off;
            // ToDo: disable vibration
            return;
        }

        switch (displayState)
        {
        case DisplayState::Clock:
        case DisplayState::ClockWithAlarmLeds:
            updateDisplayState(DisplayState::Standby);
            break;

        default:
            break;
        }
        break;
    }
    case util::Button::Action::StopLongPress:
        break;
    }
}

//-----------------------------------------------------------------
void StateMachine::buttonRightCallback(util::Button::Action action)
{
    if (alarmState != AlarmState::Off)
        return;

    switch (action)
    {
    case util::Button::Action::ShortPress:
    {
        switch (displayState)
        {
        case DisplayState::Clock:
        case DisplayState::ClockWithAlarmLeds:
            updateDisplayState(DisplayState::DisplayAlarmStatus);
            break;

        case DisplayState::DisplayAlarmStatus:
        {
            switch (alarmMode)
            {
            case AlarmMode::Off:
                alarmMode = AlarmMode::Alarm1;
                break;

            case AlarmMode::Alarm1:
                alarmMode = AlarmMode::Alarm2;
                break;

            case AlarmMode::Alarm2:
                alarmMode = AlarmMode::Both;
                break;

            case AlarmMode::Both:
                alarmMode = AlarmMode::Off;
                break;
            }
            revokeDisplayDelay();
        }
        break;

        case DisplayState::ChangeAlarm1Hour:
        case DisplayState::ChangeAlarm2Hour:
        case DisplayState::ChangeClockHour:
        case DisplayState::ChangeAlarm1Minute:
        case DisplayState::ChangeAlarm2Minute:
        case DisplayState::ChangeClockMinute:
            incrementNumber();
            revokeDisplayDelay();
            break;

        case DisplayState::Standby:
            goToDefaultState();
            break;

        default:
            break;
        }

        break;
    }
    case util::Button::Action::LongPress:
    {
        switch (displayState)
        {
        case DisplayState::ChangeAlarm1Hour:
        case DisplayState::ChangeAlarm2Hour:
        case DisplayState::ChangeAlarm1Minute:
        case DisplayState::ChangeAlarm2Minute:
        case DisplayState::ChangeClockHour:
        case DisplayState::ChangeClockMinute:
            setTimeoutAndStart(250.0_ms);

        default:
            break;
        }
        break;
    }

    case util::Button::Action::SuperLongPress:
    {
        if (displayState == DisplayState::Clock || displayState == DisplayState::ClockWithAlarmLeds)
        {
            blink = true;
            timeToModify = rtc.getClockTime();
            timeToModify.second = 0;
            updateDisplayState(DisplayState::ChangeClockHour);
        }
        break;
    }

    case util::Button::Action::StopLongPress:
        stopTimeoutTimer();
    }
}

//-----------------------------------------------------------------
void StateMachine::buttonSnoozeCallback(util::Button::Action action)
{
    switch (action)
    {
    case util::Button::Action::ShortPress:
    {
        if (alarmState == AlarmState::Vibration)
        {
            // ToDo: snooze
            return;
        }

        switch (displayState)
        {
        case DisplayState::ChangeAlarm1Hour:
        case DisplayState::ChangeAlarm2Hour:
        case DisplayState::ChangeAlarm1Minute:
        case DisplayState::ChangeAlarm2Minute:
            // do nothing
            break;

        case DisplayState::Test:
            // ToDo: abort test
            [[fallthrough]];
        case DisplayState::Standby:
        default:
            goToDefaultState();
            break;
        }

        break;
    }
    case util::Button::Action::LongPress:
    {
        if (alarmState == AlarmState::Vibration)
        {
            // ToDo: snooze
            return;
        }

        ledStrip.toggleState();
        break;
    }
    default:
        break;
    }
}

//-----------------------------------------------------------------
void StateMachine::buttonBrightnessPlusCallback(util::Button::Action action)
{
    ledStrip.incrementBrightness();
    if (displayState != DisplayState::LedCCT || displayState != DisplayState::LedBrightness)
        savePreviousState();

    updateDisplayState(DisplayState::LedBrightness);
    setTimeoutAndStart(4.0_s);
    // ToDo: alarm time hour/minute
}

//-----------------------------------------------------------------
void StateMachine::buttonBrightnessMinusCallback(util::Button::Action action)
{
    ledStrip.decrementBrightness();
    if (displayState != DisplayState::LedCCT || displayState != DisplayState::LedBrightness)
        savePreviousState();

    updateDisplayState(DisplayState::LedBrightness);
    setTimeoutAndStart(4.0_s);

    // ToDo: alarm time hour/minute
}

//-----------------------------------------------------------------
void StateMachine::buttonCCTPlusCallback(util::Button::Action action)
{
    ledStrip.incrementColorTemperature();
    if (displayState != DisplayState::LedCCT || displayState != DisplayState::LedBrightness)
        savePreviousState();

    updateDisplayState(DisplayState::LedCCT);
    setTimeoutAndStart(4.0_s);
}

//-----------------------------------------------------------------
void StateMachine::buttonCCTMinusCallback(util::Button::Action action)
{
    ledStrip.decrementColorTemperature();
    if (displayState != DisplayState::LedCCT || displayState != DisplayState::LedBrightness)
        savePreviousState();

    updateDisplayState(DisplayState::LedCCT);
    setTimeoutAndStart(4.0_s);
}

//-----------------------------------------------------------------
void StateMachine::incrementNumber()
{
    blink = false;

    switch (displayState)
    {
    case DisplayState::ChangeAlarm1Hour:
    case DisplayState::ChangeAlarm2Hour:
    case DisplayState::ChangeClockHour:
        timeToModify.addHours(1);
        break;

    case DisplayState::ChangeAlarm1Minute:
    case DisplayState::ChangeAlarm2Minute:
        timeToModify.addMinutes(5);
        break;

    case DisplayState::ChangeClockMinute:
        timeToModify.addMinutes(1);
        break;

    default:
        break;
    }
}

//-----------------------------------------------------------------
void StateMachine::decrementNumber()
{
    blink = false;

    switch (displayState)
    {
    case DisplayState::ChangeAlarm1Hour:
    case DisplayState::ChangeAlarm2Hour:
    case DisplayState::ChangeClockHour:
        timeToModify.subHours(1);
        break;

    case DisplayState::ChangeAlarm1Minute:
    case DisplayState::ChangeAlarm2Minute:
        timeToModify.subMinutes(5);
        break;

    case DisplayState::ChangeClockMinute:
        timeToModify.subMinutes(1);
        break;

    default:
        break;
    }
}
