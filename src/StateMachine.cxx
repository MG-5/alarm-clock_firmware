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
    bool blink = true;

    while (true)
    {
        switch (state)
        {
        case State::Standby:
            display.disableDisplay();
            vTaskDelay(portMAX_DELAY); // todo
            break;

        case State::Clock:
            display.showClock(clock, blink);
            blink = !blink;
            vTaskDelay(toOsTicks(1.0_s));
            break;

        case State::DisplayAlarm1:
            display.showClock(alarmTime1, true);
            statusLeds.ledAlarm1.setState(blink);
            blink = !blink;
            vTaskDelay(toOsTicks(500.0_ms));
            break;

        case State::DisplayAlarm2:
            display.showClock(alarmTime2, true);
            statusLeds.ledAlarm2.setState(blink);
            blink = !blink;
            vTaskDelay(toOsTicks(500.0_ms));
            break;

        case State::ChangeAlarm1Hour:
            handleAlarmHourChange(blink, alarmTime1, statusLeds.ledAlarm1);
            blink = !blink;
            vTaskDelay(toOsTicks(500.0_ms));
            break;

        case State::ChangeAlarm2Hour:
            handleAlarmHourChange(blink, alarmTime2, statusLeds.ledAlarm2);
            blink = !blink;
            vTaskDelay(toOsTicks(500.0_ms));
            break;

        case State::ChangeAlarm1Minute:
            handleAlarmMinuteChange(blink, alarmTime1, statusLeds.ledAlarm1);
            blink = !blink;
            vTaskDelay(toOsTicks(500.0_ms));
            break;

        case State::ChangeAlarm2Minute:
            handleAlarmMinuteChange(blink, alarmTime2, statusLeds.ledAlarm2);
            blink = !blink;
            vTaskDelay(toOsTicks(500.0_ms));
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
    ledAlarm.setState(blink);
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
    ledAlarm.setState(blink);
    if (!blink)
    {
        // replace numbers with underscore
        display.gridDataArray[3].segments = display.gridDataArray[4].segments = font.getGlyph('_');
    }
}
