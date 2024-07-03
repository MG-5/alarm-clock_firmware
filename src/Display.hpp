#pragma once

#include "Dimming.hpp"
#include "units/si/time.hpp"
#include "util/gpio.hpp"
#include "wrappers/Task.hpp"

class Display : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    Display(Dimming &dimming)
        : TaskWithMemberFunctionBase("displayTask", 256, osPriorityAboveNormal4), //
          dimming(dimming){};

    // APP = 80MHz -> 1MHz = 1µs -> prescaler 80-1
    // auto reload period = 249 -> interrupt every 250µs
    static constexpr auto MultiplexingStepPeriod = 250.0_us;

    void multiplexingStep();
    void pwmTimerInterrupt();

protected:
    void taskMain(void *) override;

private:
    Dimming &dimming;

    util::Gpio heatwire{enableHeatwire_GPIO_Port, enableHeatwire_Pin};
    util::Gpio boostConverter{enable35V_GPIO_Port, enable35V_Pin};

    // shift register
    static constexpr auto NumberBitsInShiftRegister = 17;

    util::Gpio shiftRegisterData{ShiftRegisterData_GPIO_Port, ShiftRegisterData_Pin};
    util::Gpio shiftRegisterClock{ShiftRegisterClock_GPIO_Port, ShiftRegisterClock_Pin};
    util::Gpio shiftRegisterStrobe{Strobe_GPIO_Port, Strobe_Pin};

    static constexpr auto NumberOfGrids = 6;

    std::array<util::Gpio, NumberOfGrids> gridArray //
        {util::Gpio(enableGrid0_GPIO_Port, enableGrid0_Pin),
         util::Gpio(enableGrid1_GPIO_Port, enableGrid1_Pin),
         util::Gpio(enableGrid2_GPIO_Port, enableGrid2_Pin),
         util::Gpio(enableGrid3_GPIO_Port, enableGrid3_Pin),
         util::Gpio(enableGrid4_GPIO_Port, enableGrid4_Pin),
         util::Gpio(enableGrid5_GPIO_Port, enableGrid5_Pin)};

    void setup();
    void showInitialization();

    void clockPeriod();
    void strobePeriod();
    void sendSegmentBits(uint32_t bits, bool enableDots = false, bool enableUpperBar = false,
                         bool enableLowerBar = false);

    void disableAllGrids();

    uint8_t gridIndex = 0;

    /*   A
        ----I-----
       | \  |  / |
      F|  H | J  |B
       |   \|/   |
     G1 ---- ---- G2
       |   /|\   |
      E|  M | K  |C
       | /  |  \ |
        ----L---- D */

    static constexpr std::array<uint32_t, 10> numberSegments{
        // BCDEFGGHIJKLM
        0b11111100001001, // 0
        0b01100000001000, // 1
        0b11011011000000, // 2
        0b11110001000000, // 3
        0b01100111000000, // 4
        0b10110111000000, // 5
        0b10111111000000, // 6
        0b11100000000000, // 7
        0b11111111000000, // 8
        0b11110111000000  // 9
    };
};
