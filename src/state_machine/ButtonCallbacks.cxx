#include "StateMachine.hpp"

void StateMachine::handleTimeoutTimer()
{
    switch (displayState)
    {
    case DisplayState::ChangeAlarm1Hour:
    case DisplayState::ChangeAlarm2Hour:
    case DisplayState::ChangeClockHour:
        blink = false;
        isIncrementing ? timeToModify.addHours(1) : timeToModify.subHours(1);
        break;

    case DisplayState::ChangeAlarm1Minute:
    case DisplayState::ChangeAlarm2Minute:
        blink = false;
        isIncrementing ? timeToModify.addMinutes(5) : timeToModify.subMinutes(5);
        break;

    case DisplayState::ChangeClockMinute:
        blink = false;
        isIncrementing ? timeToModify.addMinutes(1) : timeToModify.subMinutes(1);
        break;

    case DisplayState::LedBrightness:
        isIncrementing ? ledStrip.incrementBrightness() : ledStrip.decrementBrightness();
        break;

    case DisplayState::LedCCT:
        isIncrementing ? ledStrip.incrementCCT() : ledStrip.decrementCCT();
        break;

    default: // as fallback
        stopTimeoutTimer();
        return;
        break;
    }

    revokeDisplayDelay();
}

//-----------------------------------------------------------------
// LEFT
void StateMachine::buttonLeftCallback(util::Button::Action action)
{
    switch (action)
    {
    case util::Button::Action::ShortPress:
    {
        if (rtc.getAlarmState() != RealTimeClock::AlarmState::Off)
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
            rtc.setAlarmMode(RealTimeClock::AlarmMode::Alarm1);
            updateDisplayState(DisplayState::DisplayAlarm1);
            break;

        case DisplayState::ChangeAlarm2Hour:
            updateDisplayState(DisplayState::ChangeAlarm2Minute);
            break;

        case DisplayState::ChangeAlarm2Minute:
            signalResult(rtc.writeAlarmTime2(timeToModify));
            rtc.setAlarmMode(RealTimeClock::AlarmMode::Alarm2);
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
        if (rtc.getAlarmState() != RealTimeClock::AlarmState::Off)
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
        if (rtc.getAlarmState() != RealTimeClock::AlarmState::Off)
        {
            rtc.setAlarmState(RealTimeClock::AlarmState::Off);
            initialAlarm = true;
            revokeDisplayDelay();
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
// RIGHT
void StateMachine::buttonRightCallback(util::Button::Action action)
{
    if (rtc.getAlarmState() != RealTimeClock::AlarmState::Off)
        return;

    switch (action)
    {
    case util::Button::Action::ShortPress:
    {
        if (isInChangeScreen())
        {
            incrementNumber();
            revokeDisplayDelay();
            return;
        }

        switch (displayState)
        {
        case DisplayState::Clock:
        case DisplayState::ClockWithAlarmLeds:
            updateDisplayState(DisplayState::DisplayAlarmStatus);
            break;

        case DisplayState::DisplayAlarmStatus:
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
            revokeDisplayDelay();
        }
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
        if (isInChangeScreen())
            setTimeoutAndStart(250.0_ms);
        break;

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
        if (isInChangeScreen())
            stopTimeoutTimer();
    }
}

//-----------------------------------------------------------------
// SNOOZE
void StateMachine::buttonSnoozeCallback(util::Button::Action action)
{
    if (rtc.getAlarmState() != RealTimeClock::AlarmState::Off)
    {
        if (rtc.getAlarmState() == RealTimeClock::AlarmState::Vibration)
        {
            rtc.setAlarmState(RealTimeClock::AlarmState::Snooze);
            revokeDisplayDelay();
        }

        return;
    }

    switch (action)
    {
    case util::Button::Action::ShortPress:
    {
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
        ledStrip.toggleState();
        if (displayState == DisplayState::Standby)
            updateDisplayState(DisplayState::Clock);
        break;
    }

    default:
        break;
    }
}

//-----------------------------------------------------------------
void StateMachine::buttonBrightnessPlusCallback(util::Button::Action action)
{
    handlePlusMinusButtons(action, true, true);
}

//-----------------------------------------------------------------
void StateMachine::buttonBrightnessMinusCallback(util::Button::Action action)
{
    handlePlusMinusButtons(action, false, true);
}

//-----------------------------------------------------------------
void StateMachine::buttonCCTPlusCallback(util::Button::Action action)
{
    handlePlusMinusButtons(action, true, false);
}

//-----------------------------------------------------------------
void StateMachine::buttonCCTMinusCallback(util::Button::Action action)
{
    handlePlusMinusButtons(action, false, false);
}
//-----------------------------------------------------------------
bool StateMachine::isInChangeScreen()
{
    return displayState == DisplayState::ChangeAlarm1Hour || displayState == DisplayState::ChangeAlarm2Hour ||
           displayState == DisplayState::ChangeClockHour || displayState == DisplayState::ChangeAlarm1Minute ||
           displayState == DisplayState::ChangeAlarm2Minute || displayState == DisplayState::ChangeClockMinute;
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

//-----------------------------------------------------------------
void StateMachine::switchToLedChangeScreen(DisplayState newState)
{
    if (displayState != DisplayState::LedCCT && displayState != DisplayState::LedBrightness)
        savePreviousState();

    updateDisplayState(newState);
}

//-----------------------------------------------------------------
void StateMachine::handlePlusMinusButtons(util::Button::Action action, bool isPlus, bool isBrightness)
{
    if (rtc.getAlarmState() != RealTimeClock::AlarmState::Off)
        return;

    switch (action)
    {
    case util::Button::Action::ShortPress:
        if (isInChangeScreen())
        {
            isPlus ? incrementNumber() : decrementNumber();
            revokeDisplayDelay();
            return;
        }

        isPlus ? (isBrightness ? ledStrip.incrementBrightness() : ledStrip.incrementCCT())
               : (isBrightness ? ledStrip.decrementBrightness() : ledStrip.decrementCCT());
        switchToLedChangeScreen(isBrightness ? DisplayState::LedBrightness : DisplayState::LedCCT);
        break;

    case util::Button::Action::LongPress:
        if (!isInChangeScreen())
            switchToLedChangeScreen(isBrightness ? DisplayState::LedBrightness : DisplayState::LedCCT);

        isIncrementing = isPlus;
        setTimeoutAndStart(250.0_ms);
        break;

    case util::Button::Action::StopLongPress:
        stopTimeoutTimer();
        break;

    default:
        break;
    }
}
