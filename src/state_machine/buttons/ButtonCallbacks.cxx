#include "../StateMachine.hpp"

void OldStateMachine::handleTimeoutTimer()
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

    // revokeDisplayDelay();
}

//-----------------------------------------------------------------
// LEFT
void OldStateMachine::buttonLeftCallback(util::Button::Action action)
{
    switch (action)
    {
    case util::Button::Action::ShortPress:
    {
        if (rtc.getAlarmState() != RealTimeClock::AlarmState::Off)
            return;

        blink = true;
    }
    break;

    case util::Button::Action::LongPress:
    {
        if (rtc.getAlarmState() != RealTimeClock::AlarmState::Off)
            return;
    }
    break;
    case util::Button::Action::SuperLongPress:
    {
        if (rtc.getAlarmState() != RealTimeClock::AlarmState::Off)
        {
            rtc.setAlarmState(RealTimeClock::AlarmState::Off);
            // vibrationCushion.write(false);
            initialAlarm = true;
            // revokeDisplayDelay();
            return;
        }
        break;
    }
    case util::Button::Action::StopLongPress:
        break;
    }
}

//-----------------------------------------------------------------
// RIGHT
void OldStateMachine::buttonRightCallback(util::Button::Action action)
{
    if (rtc.getAlarmState() != RealTimeClock::AlarmState::Off)
        return;

    switch (action)
    {
    case util::Button::Action::LongPress:
        if (isInChangeScreen())
            setTimeoutAndStart(250.0_ms);

        else if (displayState != DisplayState::DisplayAlarm1 && displayState != DisplayState::DisplayAlarm2)
            // updateDisplayState(DisplayState::Test);
            break;

    case util::Button::Action::StopLongPress:
        if (isInChangeScreen())
            stopTimeoutTimer();
    }
}

//-----------------------------------------------------------------
// SNOOZE
void OldStateMachine::buttonSnoozeCallback(util::Button::Action action)
{
    if (rtc.getAlarmState() != RealTimeClock::AlarmState::Off)
    {
        if (rtc.getAlarmState() == RealTimeClock::AlarmState::Vibration)
        {
            rtc.setAlarmState(RealTimeClock::AlarmState::Snooze);
            // revokeDisplayDelay();
        }

        return;
    }

    switch (action)
    {
    case util::Button::Action::ShortPress:
    {
        switch (displayState)
        {
        case DisplayState::Test:
            abortTest();
            [[fallthrough]];
        default:
            // goToDefaultState();
            break;
        }

        break;
    }
    case util::Button::Action::LongPress:
    {
        isLedStripOn = !isLedStripOn;
        isLedStripOn ? ledStrip.turnOnWithFade() : ledStrip.turnOffWithFade();
        break;
    }

    default:
        break;
    }
}

//-----------------------------------------------------------------
void OldStateMachine::buttonBrightnessPlusCallback(util::Button::Action action)
{
    handlePlusMinusButtons(action, true, true);
}

//-----------------------------------------------------------------
void OldStateMachine::buttonBrightnessMinusCallback(util::Button::Action action)
{
    handlePlusMinusButtons(action, false, true);
}

//-----------------------------------------------------------------
void OldStateMachine::buttonCCTPlusCallback(util::Button::Action action)
{
    handlePlusMinusButtons(action, true, false);
}

//-----------------------------------------------------------------
void OldStateMachine::buttonCCTMinusCallback(util::Button::Action action)
{
    handlePlusMinusButtons(action, false, false);
}
//-----------------------------------------------------------------
bool OldStateMachine::isInChangeScreen()
{
    return displayState == DisplayState::ChangeAlarm1Hour || displayState == DisplayState::ChangeAlarm2Hour ||
           displayState == DisplayState::ChangeClockHour || displayState == DisplayState::ChangeAlarm1Minute ||
           displayState == DisplayState::ChangeAlarm2Minute || displayState == DisplayState::ChangeClockMinute;
}

//-----------------------------------------------------------------
void OldStateMachine::incrementNumber()
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
void OldStateMachine::decrementNumber()
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
void OldStateMachine::switchToLedChangeScreen(DisplayState newState)
{
    if (displayState != DisplayState::LedCCT && displayState != DisplayState::LedBrightness)
    { // savePreviousState();
    }

    // updateDisplayState(newState);
}

//-----------------------------------------------------------------
void OldStateMachine::handlePlusMinusButtons(util::Button::Action action, bool isPlus, bool isBrightness)
{
    if (rtc.getAlarmState() != RealTimeClock::AlarmState::Off)
        return;

    switch (action)
    {
    case util::Button::Action::ShortPress:
        if (isInChangeScreen())
        {
            isPlus ? incrementNumber() : decrementNumber();
            // revokeDisplayDelay();
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
        setTimeoutAndStart(150.0_ms);
        break;

    case util::Button::Action::StopLongPress:
        stopTimeoutTimer();
        break;

    default:
        break;
    }
}
