#include "Display.hpp"
#include "helpers/freertos.hpp"

void Display::taskMain(void *)
{
    setup();
    vTaskDelay(toOsTicks(500.0_ms));
    // showInitialization();
    // vTaskDelay(toOsTicks(2.0_s));

    boostConverter.write(true);
    dimming.initPwm(); // also starts multiplexing
    dimming.setBrightness(25);

    gridData[0] = font.getGlyph('H');
    gridData[1] = font.getGlyph('A');
    gridData[2] = font.getGlyph('L');
    gridData[3] = font.getGlyph('L');
    gridData[4] = font.getGlyph('O');

    // nothind to do here
    vTaskSuspend(nullptr);
}

//-----------------------------------------------------------------
void Display::setup()
{
    heatwire.write(true);
    sendSegmentBits(0, false, false, false);
}

//-----------------------------------------------------------------
void Display::showInitialization()
{
    for (auto &grid : gridGpioArray)
        grid.write(true);

    uint32_t bits = 1;

    while ((bits & (1 << (NumberBitsInShiftRegister - 3))) == 0)
    {
        sendSegmentBits(bits, false, false, false);
        vTaskDelay(toOsTicks(100.0_ms));

        bits <<= 1;
    }

    bits >>= 2;
    bits |= 1 << (NumberBitsInShiftRegister - 4);

    while ((bits & 1) == 0)
    {
        sendSegmentBits(bits, false, false, false);
        vTaskDelay(toOsTicks(100.0_ms));

        bits >>= 1;
        bits |= 1 << (NumberBitsInShiftRegister - 4);
    }

    sendSegmentBits(bits, false, false, false);
    vTaskDelay(toOsTicks(100.0_ms));
    sendSegmentBits(bits, true, true, true);
}

//-----------------------------------------------------------------
void Display::pwmTimerInterrupt()
{
    disableAllGrids();
}

//-----------------------------------------------------------------
void Display::clockPeriod()
{
    shiftRegisterClock.write(true);
    shiftRegisterClock.write(false);
}

//-----------------------------------------------------------------
void Display::strobePeriod()
{
    shiftRegisterStrobe.write(true);
    shiftRegisterStrobe.write(false);
}

//-----------------------------------------------------------------
void Display::sendSegmentBits(uint32_t bits, bool enableDots, bool enableUpperBar,
                              bool enableLowerBar)
{
    bits <<= 3; // shift bits to make place for N (upperBar), O_DP (dots) and P_MIN_SEC (lowerBar)
    bits |= ((enableUpperBar & 1) << 2) | ((enableDots & 1) << 1) | (enableLowerBar & 1);

    for (auto i = 0; i < NumberBitsInShiftRegister; i++)
    {
        shiftRegisterData.write((bits >> i) & 1);
        clockPeriod();
    }

    strobePeriod();
}

//-----------------------------------------------------------------
void Display::multiplexingStep()
{
    if (++gridIndex >= NumberOfGrids)
        gridIndex = 0;

    disableAllGrids();
    sendSegmentBits(gridData[gridIndex]);

    gridGpioArray[gridIndex].write(true);
}

//--------------------------------------------------------------------------------------------------
inline void Display::disableAllGrids()
{
    for (auto &grid : gridGpioArray)
        grid.write(false);
}