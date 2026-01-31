#include "StateMachine.hpp"
#include "helpers/freertos.hpp"
#include "sync.hpp"

void OldStateMachine::taskMain(void *)
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
                // updateDisplayState(DisplayState::Clock); // also wake up display
                ledStrip.turnOnWithFade();
                isLedStripOn = true;
            }

            // vibrationCushion.write(rtc.getAlarmState() == RealTimeClock::AlarmState::Vibration);

            // showClockWithBlinkingAlarm();
            // ToDo: sunrise fading
            // ToDo: control vibration
            // ToDo: turn off alarm after 2 min no reaction
            delayUntilEventOrTimeout(1.0_s);
            continue; // bypass displayState processing
        }

        // checkIfGoToStandby();
        processDisplayState();
    }
};

//-----------------------------------------------------------------
// process current display state
void OldStateMachine::processDisplayState()
{
    switch (displayState)
    {
    case DisplayState::ChangeClockHour:
        // showHourChanging();
        blink = !blink;
        break;

    case DisplayState::ChangeClockMinute:
        // showMinuteChanging();
        blink = !blink;
        break;

    case DisplayState::LedBrightness:
        showCurrentBrightness();

        break;

    case DisplayState::LedCCT:
        showCurrentCCT();

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
        // vibrationCushion.write(true);

        delayUntilEventOrTimeout(2.0_s);

        abortTest();
    }
    break;

    default:
        break;
    }
}

//-----------------------------------------------------------------
void OldStateMachine::showCurrentBrightness()
{
    display.getGridDataArray()[2].segments = font.getGlyph('B');
    display.getGridDataArray()[2].enableDots = true;

    uint8_t brightness = ledStrip.getGlobalBrightness();
    display.getGridDataArray()[3].segments = font.getGlyph('0' + brightness / 100);
    display.getGridDataArray()[4].segments = font.getGlyph('0' + (brightness % 100) / 10);
    display.getGridDataArray()[5].segments = font.getGlyph('0' + brightness % 10);
}

//-----------------------------------------------------------------
void OldStateMachine::showCurrentCCT()
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
void OldStateMachine::abortTest()
{
    ledStrip.setColorTemperature(prevColor, false);
    ledStrip.setGlobalBrightness(prevBrightness, false);

    if (!prevLedState)
        ledStrip.turnOffImmediately();
    // vibrationCushion.write(false);
    statusLeds.turnAllOff();
}