#pragma once

#include "DisplayDimming.hpp"
#include "font/Font.hpp"
#include "rtc/Time/Time.hpp"
#include "units/si/time.hpp"
#include "util/gpio.hpp"

class Display
{
public:
    Display(DisplayDimming &dimming) : dimming(dimming){};

    // APP = 80MHz -> 1MHz = 1µs -> prescaler 80-1
    // auto reload period = 249 -> interrupt every 250µs
    static constexpr auto MultiplexingStepPeriod = 250.0_us;

    void multiplexingInterrupt();
    void pwmTimerInterrupt();
    void setBrightness(uint8_t brightness);

    void setup();
    void showInitialization();

    void enableDisplay(bool startMultiplexing = true);
    void disableDisplay();

    static constexpr auto NumberOfGrids = 6;

    struct GridData
    {
        uint32_t segments = 0;
        bool enableDots = false;
        bool enableUpperBar = false;
        bool enableLowerBar = false;
    };

    std::array<GridData, NumberOfGrids> gridDataArray{};

    void setClock(Time clockToShow);
    void showClock(bool forceShowDots = false);

private:
    DisplayDimming &dimming;

    util::Gpio heatwire{enableHeatwire_GPIO_Port, enableHeatwire_Pin};
    util::Gpio boostConverter{enable35V_GPIO_Port, enable35V_Pin};

    // shift register
    static constexpr auto NumberBitsInShiftRegister = 17;

    util::Gpio shiftRegisterData{ShiftRegisterData_GPIO_Port, ShiftRegisterData_Pin};
    util::Gpio shiftRegisterClock{ShiftRegisterClock_GPIO_Port, ShiftRegisterClock_Pin};
    util::Gpio shiftRegisterStrobe{Strobe_GPIO_Port, Strobe_Pin};

    std::array<util::Gpio, NumberOfGrids> gridGpioArray //
        {util::Gpio(enableGrid0_GPIO_Port, enableGrid0_Pin),
         util::Gpio(enableGrid1_GPIO_Port, enableGrid1_Pin),
         util::Gpio(enableGrid2_GPIO_Port, enableGrid2_Pin),
         util::Gpio(enableGrid3_GPIO_Port, enableGrid3_Pin),
         util::Gpio(enableGrid4_GPIO_Port, enableGrid4_Pin),
         util::Gpio(enableGrid5_GPIO_Port, enableGrid5_Pin)};

    void clockPeriod();
    void strobePeriod();
    void sendSegmentBits(uint32_t bits, bool forceLatch = true, bool enableDots = false,
                         bool enableUpperBar = false, bool enableLowerBar = false);

    void disableAllGrids();

    uint8_t gridIndex = 0;
    Time currentTime;
};
