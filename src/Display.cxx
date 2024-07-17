#include "Display.hpp"
#include "helpers/freertos.hpp"
#include "sync.hpp"

void Display::taskMain(void *)
{
    setup();
    setBrightness(100);
    vTaskDelay(toOsTicks(500.0_ms));

    // showInitialization();
    syncEventGroup.setBits(sync::WaitForDisplayInitBit);
    // vTaskDelay(toOsTicks(1.0_s));
    syncEventGroup.setBits(sync::WaitForLedInit);

    enableDisplay();

    // nothing to do here cause interrupts will take c are of multiplexing
    vTaskSuspend(nullptr);
}

//-----------------------------------------------------------------
void Display::setup()
{
    enableDisplay(false); // without multiplexing due intialization do own stuff
    sendSegmentBits(0);
}

// -----------------------------------------------------------------
void Display::enableDisplay(bool startMultiplexing)
{
    heatwire.write(true);
    boostConverter.write(true);

    if (startMultiplexing)
        dimming.initPwm(); // also starts multiplexing
}

//-----------------------------------------------------------------
void Display::disableDisplay()
{
    dimming.stopPwm();
    heatwire.write(false);
    boostConverter.write(false);
}

//-----------------------------------------------------------------
void Display::showInitialization()
{
    for (auto &grid : gridGpioArray)
        grid.write(true);

    uint32_t bits = 1;

    while ((bits & (1 << (NumberBitsInShiftRegister - 3))) == 0)
    {
        sendSegmentBits(bits);
        vTaskDelay(toOsTicks(100.0_ms));

        bits <<= 1;
    }

    bits >>= 2;
    bits |= 1 << (NumberBitsInShiftRegister - 4);

    while ((bits & 1) == 0)
    {
        sendSegmentBits(bits);
        vTaskDelay(toOsTicks(100.0_ms));

        bits >>= 1;
        bits |= 1 << (NumberBitsInShiftRegister - 4);
    }

    sendSegmentBits(bits);
    vTaskDelay(toOsTicks(100.0_ms));
    sendSegmentBits(bits, true, true, true);
}

//-----------------------------------------------------------------
void Display::setBrightness(uint8_t brightness)
{
    dimming.setBrightness(brightness);
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
    // there is no delay between clock level changes because
    // the HAL consumes time enough to match shift register requirements
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
    sendSegmentBits(gridDataArray[gridIndex].segments, gridDataArray[gridIndex].enableDots,
                    gridDataArray[gridIndex].enableUpperBar,
                    gridDataArray[gridIndex].enableLowerBar);

    gridGpioArray[gridIndex].write(true);
}

//--------------------------------------------------------------------------------------------------
inline void Display::disableAllGrids()
{
    for (auto &grid : gridGpioArray)
        grid.write(false);
}

//--------------------------------------------------------------------------------------------------
void Display::showClock(const Clock_t &clock, bool showDots)
{
    // add '0' to get the ASCII value of the number
    gridDataArray[1].segments = font.getGlyph((clock.hours / 10) + '0');
    gridDataArray[2].segments = font.getGlyph((clock.hours % 10) + '0');
    gridDataArray[3].segments = font.getGlyph((clock.minutes / 10) + '0');
    gridDataArray[4].segments = font.getGlyph((clock.minutes % 10) + '0');

    gridDataArray[2].enableDots = showDots;
}