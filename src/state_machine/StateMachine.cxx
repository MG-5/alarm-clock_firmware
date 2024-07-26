#include "StateMachine.hpp"
#include "helpers/freertos.hpp"
#include "sync.hpp"

void StateMachine::taskMain(void *)
{
    waitForRtc();
    displayLedInitialization();

    while (true)
    {
        display.gridDataArray.fill(Display::GridData{});
        statusLeds.turnAllOff();

        // ToDo: replace it with reading general error state
        if (!rtc.isRtcOnline())
            statusLeds.ledRedGreen.setColor(util::pwm_led::DualLedColor::Red);

        switch (displayState)
        {
        case DisplayState::Standby:
            display.disableDisplay();
            vTaskDelay(portMAX_DELAY); // todo
            break;

        case DisplayState::Clock:
            display.setClock(rtc.getClockTime());
            display.showClock();
            delayUntilEventOrTimeout(1.0_s);
            break;

        case DisplayState::ClockWithAlarmLeds:
            display.setClock(rtc.getClockTime());
            display.showClock();
            statusLeds.ledAlarm1.setState(alarmMode == AlarmMode::Alarm1 ||
                                          alarmMode == AlarmMode::Both);
            statusLeds.ledAlarm2.setState(alarmMode == AlarmMode::Alarm2 ||
                                          alarmMode == AlarmMode::Both);
            delayUntilEventOrTimeout(1.0_s);
            break;

        case DisplayState::DisplayAlarm1:
            display.setClock(rtc.getAlarmTime1());
            display.showClock(true);
            statusLeds.ledAlarm1.setState(blink);
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::DisplayAlarm2:
            display.setClock(rtc.getAlarmTime2());
            display.showClock(true);
            statusLeds.ledAlarm2.setState(blink);
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::ChangeAlarm1Hour:
            handleAlarmHourChange(statusLeds.ledAlarm1);
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::ChangeAlarm2Hour:
            handleAlarmHourChange(statusLeds.ledAlarm2);
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::ChangeAlarm1Minute:
            handleAlarmMinuteChange(statusLeds.ledAlarm1);
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::ChangeAlarm2Minute:
            handleAlarmMinuteChange(statusLeds.ledAlarm2);
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::DisplayAlarmStatus:
            display.gridDataArray[2].segments = font.getGlyph('A');
            display.gridDataArray[2].enableDots = true;

            switch (alarmMode)
            {
            case AlarmMode::Off:
                display.gridDataArray[3].segments = font.getGlyph('O');
                display.gridDataArray[4].segments = font.getGlyph('f');
                display.gridDataArray[5].segments = font.getGlyph('f');
                break;

            case AlarmMode::Alarm1:
                display.gridDataArray[3].segments = font.getGlyph('1');
                statusLeds.ledAlarm1.turnOn();
                break;

            case AlarmMode::Alarm2:
                display.gridDataArray[3].segments = font.getGlyph('2');
                statusLeds.ledAlarm2.turnOn();
                break;

            case AlarmMode::Both:
                display.gridDataArray[3].segments = font.getGlyph('1');
                display.gridDataArray[4].segments = font.getGlyph('+');
                display.gridDataArray[5].segments = font.getGlyph('2');
                statusLeds.ledAlarm1.turnOn();
                statusLeds.ledAlarm2.turnOn();
                break;
            }
            if (delayUntilEventOrTimeout(3.0_s))
                displayState = DisplayState::Clock;
            break;

        default:
            break;
        }
    }
};

//-----------------------------------------------------------------
void StateMachine::displayLedInitialization()
{
    display.setup();

    /*
    display.showInitialization();
    statusLeds.ledRedGreen.setColor(util::pwm_led::DualLedColor::Orange);
    statusLeds.turnAllOn();
    vTaskDelay(toOsTicks(1.0_s));
    statusLeds.turnAllOff();
    */
    display.enableDisplay(); // start multiplexing
}

//-----------------------------------------------------------------
void StateMachine::waitForRtc()
{
    statusLeds.ledRedGreen.setColorBlinking(util::pwm_led::DualLedColor::Red, 2.0_Hz);
    syncEventGroup.waitBits(sync::RtcHasRespondedOnce, pdFALSE, pdFALSE, portMAX_DELAY);
    statusLeds.ledRedGreen.turnOff();
}

//-----------------------------------------------------------------
void StateMachine::handleAlarmHourChange(
    util::pwm_led::SingleLed<StatusLeds::NumberOfResolutionBits> &ledAlarm)
{
    display.setClock(alarmTimeToModify);
    display.showClock(true);
    ledAlarm.turnOn();
    if (!blink)
    {
        // replace numbers with underscore
        display.gridDataArray[1].segments = display.gridDataArray[2].segments = font.getGlyph('_');
    }
}

//-----------------------------------------------------------------
void StateMachine::handleAlarmMinuteChange(
    util::pwm_led::SingleLed<StatusLeds::NumberOfResolutionBits> &ledAlarm)
{
    display.setClock(alarmTimeToModify);
    display.showClock(true);
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
            alarmTimeToModify = rtc.getAlarmTime1();
            displayState = DisplayState::DisplayAlarm1;
            break;

        case DisplayState::DisplayAlarm1:
            alarmTimeToModify = rtc.getAlarmTime2();
            displayState = DisplayState::DisplayAlarm2;
            break;

        case DisplayState::DisplayAlarm2:
            displayState = DisplayState::Clock;
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
            blink = true;
            alarmTimeToModify.addHours(1);
            break;

        case DisplayState::ChangeAlarm1Minute:
        case DisplayState::ChangeAlarm2Minute:
            blink = true;
            alarmTimeToModify.addMinutes(5);
            break;

        case DisplayState::Standby:
            // ToDo: turn on system

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
void StateMachine::assignButtonCallbacks()
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
