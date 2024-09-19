#include "StateMachine.hpp"
#include "helpers/freertos.hpp"
#include "sync.hpp"

void StateMachine::taskMain(void *)
{
    waitForRtc();
    displayLedInitialization();

    while (true)
    {
        display.clearGridDataArray();
        statusLeds.ledAlarm1.turnOff();
        statusLeds.ledAlarm2.turnOff();

        // ToDo: replace it with reading general error state
        if (!rtc.isRtcOnline())
            statusLeds.ledRedGreen.setColor(util::led::pwm::DualLedColor::Red);

        switch (displayState)
        {
        case DisplayState::Standby:
            display.disableDisplay();
            delayUntilEventOrTimeout(0.0_s, true);
            break;

        case DisplayState::Clock:
            display.setClock(rtc.getClockTime());
            display.showClock();
            delayUntilEventOrTimeout(1.0_s);
            break;

        case DisplayState::ClockWithAlarmLeds:
            display.setClock(rtc.getClockTime());
            display.showClock();
            statusLeds.ledAlarm1.setState(rtc.getAlarmMode() == RealTimeClock::AlarmMode::Alarm1 ||
                                          rtc.getAlarmMode() == RealTimeClock::AlarmMode::Both);
            statusLeds.ledAlarm2.setState(rtc.getAlarmMode() == RealTimeClock::AlarmMode::Alarm2 ||
                                          rtc.getAlarmMode() == RealTimeClock::AlarmMode::Both);
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
            showHourChanging();
            statusLeds.ledAlarm1.turnOn();
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::ChangeAlarm2Hour:
            showHourChanging();
            statusLeds.ledAlarm2.turnOn();
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::ChangeAlarm1Minute:
            showMinuteChanging();
            statusLeds.ledAlarm1.turnOn();
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::ChangeAlarm2Minute:
            showMinuteChanging();
            statusLeds.ledAlarm2.turnOn();
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::DisplayAlarmStatus:
            showCurrentAlarmMode();
            if (delayUntilEventOrTimeout(3.0_s))
                goToDefaultState();
            break;

        case DisplayState::ChangeClockHour:
            showHourChanging();
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::ChangeClockMinute:
            showMinuteChanging();
            blink = !blink;
            delayUntilEventOrTimeout(500.0_ms);
            break;

        case DisplayState::LedBrightness:
            showCurrentBrightness();
            if (delayUntilEventOrTimeout(4.0_s))
                restorePreviousState();
            break;

        case DisplayState::LedCCT:
            showCurrentCCT();
            if (delayUntilEventOrTimeout(4.0_s))
                restorePreviousState();
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
    statusLeds.ledRedGreen.setColorBlinking(util::led::pwm::DualLedColor::Red, 2.0_Hz);
    syncEventGroup.waitBits(sync::RtcHasRespondedOnce, pdFALSE, pdFALSE, portMAX_DELAY);
    statusLeds.ledRedGreen.turnOff();
}

//-----------------------------------------------------------------
void StateMachine::showHourChanging()
{
    display.setClock(timeToModify);
    display.showClock(true);
    if (blink)
    {
        // replace numbers with underscore
        display.getGridDataArray()[1].segments = display.getGridDataArray()[2].segments = font.getGlyph('_');
    }
}

//-----------------------------------------------------------------
void StateMachine::showMinuteChanging()
{
    display.setClock(timeToModify);
    display.showClock(true);
    if (blink)
    {
        // replace numbers with underscore
        display.getGridDataArray()[3].segments = display.getGridDataArray()[4].segments = font.getGlyph('_');
    }
}

//-----------------------------------------------------------------
void StateMachine::showCurrentAlarmMode()
{
    display.getGridDataArray()[2].segments = font.getGlyph('A');
    display.getGridDataArray()[2].enableDots = true;

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
}

//-----------------------------------------------------------------
void StateMachine::showCurrentBrightness()
{
    display.getGridDataArray()[2].segments = font.getGlyph('B');
    display.getGridDataArray()[2].enableDots = true;

    uint8_t brightness = ledStrip.getGlobalBrightness();
    display.getGridDataArray()[3].segments = font.getGlyph('0' + brightness / 100);
    display.getGridDataArray()[4].segments = font.getGlyph('0' + (brightness % 100) / 10);
    display.getGridDataArray()[5].segments = font.getGlyph('0' + brightness % 10);
}

//-----------------------------------------------------------------
void StateMachine::showCurrentCCT()
{
    const uint16_t Cct = ledStrip.getColorTemperature().getMagnitude<uint16_t>();

    display.getGridDataArray()[1].segments = font.getGlyph('0' + Cct / 1000);
    display.getGridDataArray()[1].segments = font.getGlyph('0' + Cct / 1000);
    display.getGridDataArray()[2].segments = font.getGlyph('0' + (Cct % 1000) / 100);
    display.getGridDataArray()[3].segments = font.getGlyph('0' + (Cct % 100) / 10);
    display.getGridDataArray()[4].segments = font.getGlyph('0' + Cct % 10);
    display.getGridDataArray()[5].segments = font.getGlyph('K');
}

//-----------------------------------------------------------------
void StateMachine::updateDisplayState(DisplayState newState)
{
    if (displayState == DisplayState::Standby)
        display.enableDisplay();

    displayState = newState;

    if (displayState == DisplayState::Standby)
        display.disableDisplay();

    stopTimeoutTimer();
    revokeDisplayDelay();
}

//-----------------------------------------------------------------
void StateMachine::signalResult(bool success)
{
    success ? statusLeds.signalSuccess() : statusLeds.signalError();
}

//-----------------------------------------------------------------
void StateMachine::revokeDisplayDelay()
{
    notify(1, util::wrappers::NotifyAction::SetBits);
}

//-----------------------------------------------------------------
void StateMachine::goToDefaultState()
{
    updateDisplayState(DisplayState::ClockWithAlarmLeds);
    setTimeoutAndStart(4.0_s);
}

//-----------------------------------------------------------------
void StateMachine::savePreviousState()
{
    previousDisplayState = displayState;
}

//-----------------------------------------------------------------
void StateMachine::restorePreviousState()
{
    updateDisplayState(previousDisplayState);
}

//-----------------------------------------------------------------
void StateMachine::assignButtonCallbacks()
{
    buttons.left.setCallback(std::bind(&StateMachine::buttonLeftCallback, this, std::placeholders::_1));
    buttons.right.setCallback(std::bind(&StateMachine::buttonRightCallback, this, std::placeholders::_1));
    buttons.snooze.setCallback(std::bind(&StateMachine::buttonSnoozeCallback, this, std::placeholders::_1));
    buttons.brightnessPlus.setCallback(
        std::bind(&StateMachine::buttonBrightnessPlusCallback, this, std::placeholders::_1));
    buttons.brightnessMinus.setCallback(
        std::bind(&StateMachine::buttonBrightnessMinusCallback, this, std::placeholders::_1));
    buttons.cctPlus.setCallback(std::bind(&StateMachine::buttonCCTPlusCallback, this, std::placeholders::_1));
    buttons.cctMinus.setCallback(std::bind(&StateMachine::buttonCCTMinusCallback, this, std::placeholders::_1));
}