#include "WakeupController.hpp"

void WakeupController::taskMain(void *)
{
    while (true)
    {
        if (delayUntilEventOrTimeout(currentTimeout.value_or(24.0_h)))
        {
            // timeout occurred
            handleEvent(WakeupEvent::Timeout);
        }
        else
        {
            // external event occurred
            if (lastEvent.has_value())
            {
                handleEvent(lastEvent.value());
                lastEvent.reset();
            }
        }
    }
}

//-----------------------------------------------------------------
void WakeupController::triggerAlarm1()
{
    if (state != WakeupState::Idle)
        return;

    lastEvent = WakeupEvent::Alarm1Triggered;
    abortDelayWaiting();
}

//-----------------------------------------------------------------
void WakeupController::triggerAlarm2()
{
    if (state != WakeupState::Idle)
        return;

    lastEvent = WakeupEvent::Alarm2Triggered;
    abortDelayWaiting();
}

//-----------------------------------------------------------------
void WakeupController::handleButtonEvent(Buttons::ButtonId buttonId, util::Button::Action action)
{
    if (buttonId == Buttons::ButtonId::Snooze && action == util::Button::Action::ShortPress)
    {
        lastEvent = WakeupEvent::SnoozeButtonPressed;
        abortDelayWaiting();
    }
    else if (buttonId == Buttons::ButtonId::Left && action == util::Button::Action::LongPress)
    {
        lastEvent = WakeupEvent::StopButtonPressed;
        abortDelayWaiting();
    }
}

//-----------------------------------------------------------------
void WakeupController::handleEvent(WakeupEvent event)
{
    switch (state)
    {
    case WakeupState::Idle:
        if (event == WakeupEvent::Alarm1Triggered || event == WakeupEvent::Alarm2Triggered)
        {
            transitionTo(WakeupState::Sunrise);

            // let status LED blink to related alarm
            if (WakeupEvent::Alarm1Triggered == event)
                statusLeds.ledAlarm1.setBlinking(1.0_Hz);
            else
                statusLeds.ledAlarm2.setBlinking(1.0_Hz);
        }
        break;

    case WakeupState::Sunrise:
        if (event == WakeupEvent::StopButtonPressed)
            transitionTo(WakeupState::Idle);

        else if (event == WakeupEvent::Timeout)
            handleSunrise();

        break;

    case WakeupState::Waking:
        if (event == WakeupEvent::StopButtonPressed)
            transitionTo(WakeupState::Idle);

        else if (event == WakeupEvent::SnoozeButtonPressed)
            transitionTo(WakeupState::Snoozing);

        else if (event == WakeupEvent::Timeout)
            // after 1 minute of Waking state, if no button is pressed, enter AggressiveWaking state
            transitionTo(WakeupState::AggressiveWaking);

        break;

    case WakeupState::Snoozing:
        if (event == WakeupEvent::StopButtonPressed)
            transitionTo(WakeupState::Idle);

        else if (event == WakeupEvent::Timeout)
            transitionTo(WakeupState::Waking);

        break;

    case WakeupState::AggressiveWaking:
        if (event == WakeupEvent::StopButtonPressed)
            transitionTo(WakeupState::Idle);

        else if (event == WakeupEvent::SnoozeButtonPressed)
            transitionTo(WakeupState::Snoozing);

        else if (event == WakeupEvent::Timeout)
            // after 2 minutes of AggressiveWaking state, if no button is pressed, enter Idle state
            transitionTo(WakeupState::Idle);

        break;
    }
}

//-----------------------------------------------------------------
void WakeupController::transitionTo(WakeupState newState)
{
    state = newState;

    switch (state)
    {
    case WakeupState::Idle:
        vibrationController.stopVibration();
        ledStrip.stopBlinking();
        currentTimeout.reset();
        break;

    case WakeupState::Sunrise:
        sunriseElapsedSeconds = 0;
        currentTimeout = SunriseRefreshRate;
        break;

    case WakeupState::Waking:
        vibrationController.startVibration(0.25_Hz); // two seconds on, two seconds off
        currentTimeout = 1.0_min;
        break;

    case WakeupState::Snoozing:
        vibrationController.stopVibration();
        ledStrip.stopBlinking();
        currentTimeout = 5.0_min;
        break;

    case WakeupState::AggressiveWaking:
        vibrationController.startVibration(1.0_Hz); // 500ms on, 500ms off
        ledStrip.setBlinking(2.0_Hz);               // 250ms on, 250ms off
        currentTimeout = 2.0_min;
        break;
    }
}

//-----------------------------------------------------------------
void WakeupController::handleSunrise()
{
    const uint32_t PwmMaximumValue = ledStrip.getMaximumPwmValue();
    const uint32_t SunriseDurationSeconds = static_cast<uint32_t>(SunriseDuration.getMagnitude());
    const uint32_t HalfSunriseDurationSeconds = SunriseDurationSeconds / 2;

    // split sunrise into two phases: first half for warm white, second half for cold white incrementation
    if (sunriseElapsedSeconds <= HalfSunriseDurationSeconds)
    {
        // Phase 1: Fade in warm white.
        const size_t warmWhiteLevel = (sunriseElapsedSeconds * PwmMaximumValue) / HalfSunriseDurationSeconds;

        ledStrip.setRawWarmWhiteLevel(warmWhiteLevel);
        ledStrip.setRawColdWhiteLevel(0);
    }
    else if (sunriseElapsedSeconds < SunriseDurationSeconds)
    {
        // Phase 2: Keep warm white at maximum and fade in cold white.
        const uint32_t coldWhiteElapsed = sunriseElapsedSeconds - HalfSunriseDurationSeconds;

        const size_t coldWhiteLevel = (coldWhiteElapsed * PwmMaximumValue) / HalfSunriseDurationSeconds;

        ledStrip.setRawWarmWhiteLevel(PwmMaximumValue);
        ledStrip.setRawColdWhiteLevel(coldWhiteLevel);
    }
    else
    {
        // Sunrise finished.
        ledStrip.setRawWarmWhiteLevel(PwmMaximumValue);
        ledStrip.setRawColdWhiteLevel(PwmMaximumValue);

        transitionTo(WakeupState::Waking);
        return;
    }

    sunriseElapsedSeconds++;
}
