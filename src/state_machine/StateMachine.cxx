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

        // check if alarm is activated
        if (rtc.getAlarmState() != RealTimeClock::AlarmState::Off)
        {
            if (initialAlarm)
            {
                initialAlarm = false;
                alarmStateCounter = 0;
                updateDisplayState(DisplayState::Clock); // also wake up display
                ledStrip.turnOnWithFade();
                isLedStripOn = true;
            }

            vibrationCushion.write(rtc.getAlarmState() == RealTimeClock::AlarmState::Vibration);

            showClockWithBlinkingAlarm();
            // ToDo: sunrise fading
            // ToDo: control vibration
            // ToDo: turn off alarm after 2 min no reaction
            delayUntilEventOrTimeout(1.0_s);
            continue; // bypass displayState processing
        }

        checkIfGoToStandby();
        processDisplayState();
    }
};

// -----------------------------------------------------------------
/// draw display with clock and blinking alarm LEDs
void StateMachine::showClockWithBlinkingAlarm()
{
    auto currentClockTime = rtc.getClockTime();
    display.setClock(currentClockTime);
    display.showClock();
    bool shouldBlink = currentClockTime.second % 2 == 0;
    statusLeds.ledAlarm1.setState(shouldBlink && (rtc.getAlarmMode() == RealTimeClock::AlarmMode::Alarm1 ||
                                                  rtc.getAlarmMode() == RealTimeClock::AlarmMode::Both));
    statusLeds.ledAlarm2.setState(shouldBlink && (rtc.getAlarmMode() == RealTimeClock::AlarmMode::Alarm2 ||
                                                  rtc.getAlarmMode() == RealTimeClock::AlarmMode::Both));
}

//-----------------------------------------------------------------
void StateMachine::checkIfGoToStandby()
{
    // ToDo: wake up after 7:00

    if (displayState == DisplayState::Clock && !isLedStripOn)
    {
        // go to standby between 23:00 and 7:00
        if (rtc.getClockTime().hour >= 23 || rtc.getClockTime().hour < 7)
        {
            if (secondsCounter++ >= 10)
            {
                secondsCounter = 0;
                updateDisplayState(DisplayState::Standby);
            }
        }
    }
}

//-----------------------------------------------------------------
void StateMachine::processDisplayState()
{
    switch (displayState)
    {
    case DisplayState::Standby:
        delayUntilEventOrTimeout(1.0_s);
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
        if (delayUntilEventOrTimeout(1.0_s))
            if (secondsCounter++ >= 3)
            {
                secondsCounter = 0;
                updateDisplayState(DisplayState::Clock);
            }
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

    case DisplayState::Test:
    {
        for (size_t i = 0; i < display.getGridDataArray().size(); i++)
        {
            display.getGridDataArray()[i].segments = 0b11111111111111; // 15 segments
            display.getGridDataArray()[i].enableDots = true;
            display.getGridDataArray()[i].enableUpperBar = true;
            display.getGridDataArray()[i].enableLowerBar = true;
        }
        statusLeds.ledRedGreen.setColor(util::led::pwm::DualLedColor::Yellow);
        statusLeds.turnAllOn();
        prevLedState = isLedStripOn;
        prevColor = ledStrip.getColorTemperature();
        prevBrightness = ledStrip.getGlobalBrightness();
        ledStrip.setColorTemperature(LedStrip::NeutralColorTemperature, false);
        ledStrip.setGlobalBrightness(100, false);
        vibrationCushion.write(true);

        delayUntilEventOrTimeout(2.0_s);

        abortTest();
    }
    break;

    default:
        break;
    }
}

//-----------------------------------------------------------------
void StateMachine::displayLedInitialization()
{
    statusLeds.turnAllOff();
    display.setup();
    display.showInitialization();
    statusLeds.ledRedGreen.setColor(util::led::pwm::DualLedColor::Orange);
    statusLeds.turnAllOn();
    vibrationCushion.write(true);
    vTaskDelay(toOsTicks(1.0_s));
    statusLeds.turnAllOff();
    vibrationCushion.write(false);

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
    if (newState == DisplayState::Standby)
        display.disableDisplay();

    else if (displayState == DisplayState::Standby)
        display.enableDisplay();

    displayState = newState;

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
    secondsCounter = 0;
    updateDisplayState(DisplayState::ClockWithAlarmLeds);
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
void StateMachine::abortTest()
{
    ledStrip.setColorTemperature(prevColor, false);
    ledStrip.setGlobalBrightness(prevBrightness, false);

    if (!prevLedState)
        ledStrip.turnOffImmediately();
    vibrationCushion.write(false);
    statusLeds.turnAllOff();
    updateDisplayState(DisplayState::Clock);
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