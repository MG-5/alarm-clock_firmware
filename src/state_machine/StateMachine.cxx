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
            // ToDo: implement correct behavior
            vTaskDelay(portMAX_DELAY);
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
            showCurrentAlarmMode();
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
void StateMachine::handleAlarmHourChange(StatusLeds::SingleLed &ledAlarm)
{
    display.setClock(alarmTimeToModify);
    display.showClock(true);
    ledAlarm.turnOn();
    if (blink)
    {
        // replace numbers with underscore
        display.gridDataArray[1].segments = display.gridDataArray[2].segments = font.getGlyph('_');
    }
}

//-----------------------------------------------------------------
void StateMachine::handleAlarmMinuteChange(StatusLeds::SingleLed &ledAlarm)
{
    display.setClock(alarmTimeToModify);
    display.showClock(true);
    ledAlarm.turnOn();
    if (blink)
    {
        // replace numbers with underscore
        display.gridDataArray[3].segments = display.gridDataArray[4].segments = font.getGlyph('_');
    }
}

//-----------------------------------------------------------------
void StateMachine::showCurrentAlarmMode()
{
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
        display.gridDataArray[4].segments = font.getGlyph('1');
        statusLeds.ledAlarm1.turnOn();
        break;

    case AlarmMode::Alarm2:
        display.gridDataArray[4].segments = font.getGlyph('2');
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