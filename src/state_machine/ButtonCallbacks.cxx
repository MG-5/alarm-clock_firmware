#include "StateMachine.hpp"

void StateMachine::handleTimeoutTimer()
{
    switch (displayState)
    {
    case DisplayState::ClockWithAlarmLeds:
        stopTimeoutTimer();
        displayState = DisplayState::Clock;
        break;

    case DisplayState::ChangeAlarm1Hour:
    case DisplayState::ChangeAlarm2Hour:
    case DisplayState::ChangeClockHour:
        blink = false;
        timeToModify.addHours(1);
        break;

    case DisplayState::ChangeAlarm1Minute:
    case DisplayState::ChangeAlarm2Minute:
        blink = false;
        timeToModify.addMinutes(5);
        break;

    case DisplayState::ChangeClockMinute:
        blink = false;
        timeToModify.addMinutes(1);
        break;

    default: // as fallback
        stopTimeoutTimer();
        break;
    }

    notify(1, util::wrappers::NotifyAction::SetBits);
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
            displayState = DisplayState::DisplayAlarm1;
            break;

        case DisplayState::DisplayAlarm1:
            timeToModify = rtc.getAlarmTime2();
            displayState = DisplayState::DisplayAlarm2;
            break;

        case DisplayState::DisplayAlarm2:
            goToDefaultState();
            break;

        case DisplayState::ChangeAlarm1Hour:
            displayState = DisplayState::ChangeAlarm1Minute;
            break;

        case DisplayState::ChangeAlarm1Minute:
            signalResult(rtc.writeAlarmTime1(timeToModify));
            alarmMode = AlarmMode::Alarm1;
            displayState = DisplayState::DisplayAlarm1;
            break;

        case DisplayState::ChangeAlarm2Hour:
            displayState = DisplayState::ChangeAlarm2Minute;
            break;

        case DisplayState::ChangeAlarm2Minute:
            signalResult(rtc.writeAlarmTime2(timeToModify));
            alarmMode = AlarmMode::Alarm2;
            displayState = DisplayState::DisplayAlarm2;
            break;

        case DisplayState::ChangeClockHour:
            displayState = DisplayState::ChangeClockMinute;
            break;

        case DisplayState::ChangeClockMinute:
            signalResult(rtc.writeClockTime(timeToModify));
            goToDefaultState();
            break;

        case DisplayState::Standby:
            // ToDo: turn on system
            break;

        default:
            break;
        }

        blink = true;
        notify(1, util::wrappers::NotifyAction::SetBits);
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
            displayState = DisplayState::ChangeAlarm1Hour;
            break;

        case DisplayState::DisplayAlarm2:
            blink = true;
            displayState = DisplayState::ChangeAlarm2Hour;
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
        }

        switch (displayState)
        {
        case DisplayState::Clock:
        case DisplayState::ClockWithAlarmLeds:
            // ToDo: go to standby
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
            displayState = DisplayState::DisplayAlarmStatus;
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
        }
        break;

        case DisplayState::ChangeAlarm1Hour:
        case DisplayState::ChangeAlarm2Hour:
        case DisplayState::ChangeClockHour:
        case DisplayState::ChangeAlarm1Minute:
        case DisplayState::ChangeAlarm2Minute:
        case DisplayState::ChangeClockMinute:
            incrementNumber();
            break;

        case DisplayState::Standby:
            // ToDo: turn on system
            break;

        default:
            break;
        }

        notify(1, util::wrappers::NotifyAction::SetBits);

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
            setTimeoutTimerPeriod(250.0_ms);
            resetTimeoutTimer();

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
            displayState = DisplayState::ChangeClockHour;
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

        // ToDo: toggle LED strip
        break;
    }
    default:
        break;
    }
}

//-----------------------------------------------------------------
void StateMachine::buttonBrightnessPlusCallback(util::Button::Action action)
{
    // ToDo: increment LED brightness or alarm time hour/minute
}

//-----------------------------------------------------------------
void StateMachine::buttonBrightnessMinusCallback(util::Button::Action action)
{
    // ToDo: decrement LED brightness or alarm time hour/minute
}

//-----------------------------------------------------------------
void StateMachine::buttonCCTPlusCallback(util::Button::Action action)
{
    // ToDo: increment LED color temperature
}

//-----------------------------------------------------------------
void StateMachine::buttonCCTMinusCallback(util::Button::Action action)
{
    // ToDo: decrement LED color temperature
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