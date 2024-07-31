#include "StateMachine.hpp"

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
            alarmTimeToModify = rtc.getAlarmTime1();
            displayState = DisplayState::DisplayAlarm1;
            break;

        case DisplayState::DisplayAlarm1:
            alarmTimeToModify = rtc.getAlarmTime2();
            displayState = DisplayState::DisplayAlarm2;
            break;

        case DisplayState::DisplayAlarm2:
            resetTimeoutTimer();
            displayState = DisplayState::ClockWithAlarmLeds;
            break;

        case DisplayState::ChangeAlarm1Hour:
            displayState = DisplayState::ChangeAlarm1Minute;
            break;

        case DisplayState::ChangeAlarm1Minute:
            rtc.writeAlarmTime1(alarmTimeToModify);
            alarmMode = AlarmMode::Alarm1;
            displayState = DisplayState::DisplayAlarm1;
            break;

        case DisplayState::ChangeAlarm2Hour:
            displayState = DisplayState::ChangeAlarm2Minute;
            break;

        case DisplayState::ChangeAlarm2Minute:
            rtc.writeAlarmTime2(alarmTimeToModify);
            alarmMode = AlarmMode::Alarm2;
            displayState = DisplayState::DisplayAlarm2;
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
            displayState = DisplayState::ChangeAlarm1Hour;
            break;

        case DisplayState::DisplayAlarm2:
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
            blink = false;
            alarmTimeToModify.addHours(1);
            break;

        case DisplayState::ChangeAlarm1Minute:
        case DisplayState::ChangeAlarm2Minute:
            blink = false;
            alarmTimeToModify.addMinutes(5);
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
            // ToDo: start timer which increase alarm hour repeately
            break;

        case DisplayState::ChangeAlarm1Minute:
        case DisplayState::ChangeAlarm2Minute:
            // ToDo: start timer which increase alarm minute repeately

        case DisplayState::Clock:
        case DisplayState::ClockWithAlarmLeds:
            // ToDo: go to standby
        default:
            break;
        }
        break;
    }
    case util::Button::Action::StopLongPress:
        // ToDo: stop timer

    default:
        break;
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
            // return to clock
            resetTimeoutTimer();
            displayState = DisplayState::ClockWithAlarmLeds;
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