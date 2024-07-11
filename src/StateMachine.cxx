#include "StateMachine.hpp"
#include "helpers/freertos.hpp"
#include "sync.hpp"

void StateMachine::taskMain(void *)
{
    syncEventGroup.waitBits(sync::WaitForDisplayInitBit, pdFALSE, pdFALSE, portMAX_DELAY);

    statusLeds.ledRedGreen.setColor(util::pwm_led::DualLedColor::Orange);
    statusLeds.turnAllOn();

    syncEventGroup.waitBits(sync::WaitForLedInit, pdFALSE, pdFALSE, portMAX_DELAY);
    statusLeds.turnAllOff();

    Clock_t clock = {18, 45, 0};
    Clock_t alarmTime1 = {11, 00, 0};
    Clock_t alarmTime2 = {10, 30, 0};

    while (true)
    {
        switch (displayState)
        {
        case DisplayState::Standby:
            display.disableDisplay();
            vTaskDelay(portMAX_DELAY); // todo
            break;

        case DisplayState::Clock:
            display.showClock(clock, blink);
            statusLeds.ledAlarm1.turnOff();
            statusLeds.ledAlarm2.turnOff();
            blink = !blink;
            delayUntilEventOrTimeout(1.0_s);
            break;

        case DisplayState::ClockWithAlarmLeds:
            display.showClock(clock, blink);
            statusLeds.ledAlarm1.setState(alarmMode == AlarmMode::Alarm1 ||
                                          alarmMode == AlarmMode::Both);
            statusLeds.ledAlarm2.setState(alarmMode == AlarmMode::Alarm2 ||
                                          alarmMode == AlarmMode::Both);
            blink = !blink;
            delayUntilEventOrTimeout(1.0_s);
            break;

        case DisplayState::DisplayAlarm1:
            display.showClock(alarmTime1, true);
            statusLeds.ledAlarm1.setState(blink);
            statusLeds.ledAlarm2.turnOff();
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::DisplayAlarm2:
            display.showClock(alarmTime2, true);
            statusLeds.ledAlarm1.turnOff();
            statusLeds.ledAlarm2.setState(blink);
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::ChangeAlarm1Hour:
            handleAlarmHourChange(blink, alarmTime1, statusLeds.ledAlarm1);
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::ChangeAlarm2Hour:
            handleAlarmHourChange(blink, alarmTime2, statusLeds.ledAlarm2);
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::ChangeAlarm1Minute:
            handleAlarmMinuteChange(blink, alarmTime1, statusLeds.ledAlarm1);
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::ChangeAlarm2Minute:
            handleAlarmMinuteChange(blink, alarmTime2, statusLeds.ledAlarm2);
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        default:
            break;
        }
    }
};

//-----------------------------------------------------------------
void StateMachine::handleAlarmHourChange(
    bool blink, Clock_t &alarmTime,
    util::pwm_led::SingleLed<StatusLeds::NumberOfResolutionBits> &ledAlarm)
{
    display.showClock(alarmTime, true);
    ledAlarm.turnOn();
    if (!blink)
    {
        // replace numbers with underscore
        display.gridDataArray[1].segments = display.gridDataArray[2].segments = font.getGlyph('_');
    }
}

//-----------------------------------------------------------------
void StateMachine::handleAlarmMinuteChange(
    bool blink, Clock_t &alarmTime,
    util::pwm_led::SingleLed<StatusLeds::NumberOfResolutionBits> &ledAlarm)
{
    display.showClock(alarmTime, true);
    ledAlarm.turnOn();
    if (!blink)
    {
        // replace numbers with underscore
        display.gridDataArray[3].segments = display.gridDataArray[4].segments = font.getGlyph('_');
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
            displayState = DisplayState::DisplayAlarm1;
            // ToDo: read RTC alarm 1 time
            break;

        case DisplayState::DisplayAlarm1:
            displayState = DisplayState::DisplayAlarm2;
            // ToDo: read RTC alarm 2 time
            break;

        case DisplayState::DisplayAlarm2:
            displayState = DisplayState::Clock;
            break;

        case DisplayState::ChangeAlarm1Hour:
            displayState = DisplayState::ChangeAlarm1Minute;
            break;

        case DisplayState::ChangeAlarm1Minute:
            displayState = DisplayState::DisplayAlarm1;
            break;

        case DisplayState::ChangeAlarm2Hour:
            displayState = DisplayState::ChangeAlarm2Minute;
            break;

        case DisplayState::ChangeAlarm2Minute:
            displayState = DisplayState::DisplayAlarm2;
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
    }
    break;

    case util::Button::Action::SuperLongPress:
        if (alarmState != AlarmState::Off)
        {
            alarmState = AlarmState::Off;
            // ToDo: turn off LEDs
            // ToDo: disable vibration
        }
        break;

    case util::Button::Action::StopLongPress:
        // ToDo: restart timeout timer
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
            // ToDo: increment alarm hour
            break;

        case DisplayState::ChangeAlarm1Minute:
        case DisplayState::ChangeAlarm2Minute:
            // ToDo: increment alarm minute
            break;

        case DisplayState::Standby:
            // ToDo: turn on system

        default:
            break;
        }
        break;

    case util::Button::Action::LongPress:
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
            // ToDo: go t
        default:
            break;
        }
        break;

    case util::Button::Action::StopLongPress:
        // ToDo: restart timeout timer

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
            displayState = DisplayState::ClockWithAlarmLeds;
            break;
        }
        break;

    case util::Button::Action::LongPress:

        if (alarmState == AlarmState::Vibration)
        {
            // ToDo: snooze
            return;
        }

        // ToDo: toggle LED strip
        break;

    default:
        break;
    }
}

//-----------------------------------------------------------------
void StateMachine::buttonBrightnessPlusCallback(util::Button::Action action)
{
}

//-----------------------------------------------------------------
void StateMachine::buttonBrightnessMinusCallback(util::Button::Action action)
{
}

//-----------------------------------------------------------------
void StateMachine::buttonCCTPlusCallback(util::Button::Action action)
{
}

//-----------------------------------------------------------------
void StateMachine::buttonCCTMinusCallback(util::Button::Action action)
{
}

//-----------------------------------------------------------------
void StateMachine::setButtonCallbacks()
{
    buttons.left.setCallback(
        std::bind(&StateMachine::buttonLeftCallback, this, std::placeholders::_1));

    buttons.right.setCallback(
        std::bind(&StateMachine::buttonRightCallback, this, std::placeholders::_1));

    buttons.snooze.setCallback(
        std::bind(&StateMachine::buttonSnoozeCallback, this, std::placeholders::_1));

    buttons.brightnessPlus.setCallback(
        std::bind(&StateMachine::buttonBrightnessPlusCallback, this, std::placeholders::_1));

    buttons.brightnessMinus.setCallback(
        std::bind(&StateMachine::buttonBrightnessMinusCallback, this, std::placeholders::_1));

    buttons.cctPlus.setCallback(
        std::bind(&StateMachine::buttonCCTPlusCallback, this, std::placeholders::_1));

    buttons.cctMinus.setCallback(
        std::bind(&StateMachine::buttonCCTMinusCallback, this, std::placeholders::_1));
}
