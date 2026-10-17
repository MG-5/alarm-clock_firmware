#pragma once

#include "hardware/LED/LedStrip.hpp"
#include "hardware/LED/StatusLeds.hpp"
#include "hardware/buttons/Buttons.hpp"
#include "hardware/vibration/VibrationController.hpp"

#include "wrappers/Task.hpp"

#include <optional>

class WakeupController : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    WakeupController(StatusLeds &statusLeds, LedStrip &ledStrip)
        : TaskWithMemberFunctionBase("wakeupControllerTask", 128, osPriorityNormal2), //
          statusLeds(statusLeds), ledStrip(ledStrip)
    {
    }

    enum class WakeupState
    {
        Idle,
        Sunrise,
        Waking,
        AggressiveWaking,
        Snoozing
    };

    enum class WakeupEvent
    {
        Alarm1Triggered,
        Alarm2Triggered,
        SnoozeButtonPressed,
        StopButtonPressed,
        Timeout
    };

    bool isActive() const
    {
        return state != WakeupState::Idle;
    }

    void triggerAlarm1();
    void triggerAlarm2();
    void handleButtonEvent(Buttons::ButtonId buttonId, util::Button::Action action);

protected:
    void taskMain(void *) override;

private:
    StatusLeds &statusLeds;
    LedStrip &ledStrip;
    VibrationController vibrationController;

    WakeupState state = WakeupState::Idle;
    std::optional<WakeupEvent> lastEvent;

    std::optional<units::si::Time> currentTimeout;

    void handleEvent(WakeupEvent event);
    void transitionTo(WakeupState newState);
    void handleSunrise();
    void handleWaking();
    void handleAggressiveWaking();

    /// block task for specified time but can be unblocked by external event e.g. button press
    /// @return true if timeout is occurred
    bool delayUntilEventOrTimeout(units::si::Time blockTime)
    {
        clearNotifications();
        return !notifyWait(UINT32_MAX, UINT32_MAX, (uint32_t *)0, toOsTicks(blockTime));
    }

    // abort delayUntilEventOrTimeout function by notifying the task
    void abortDelayWaiting()
    {
        notifyGive();
    }

    // sunrise parameters
    constexpr static units::si::Time SunriseRefreshRate = 1.0_s;
    constexpr static units::si::Time SunriseDuration = 30.0_min;

    uint32_t sunriseElapsedSeconds = 0;
};