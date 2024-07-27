#include "Display.hpp"
#include "helpers/freertos.hpp"
#include "sync.hpp"

//-----------------------------------------------------------------
void Display::setup()
{
    enableDisplay(false); // without multiplexing due intialization do its stuff
    sendSegmentBits(0);
    setBrightness(100);
    vTaskDelay(toOsTicks(500.0_ms));
}

// -----------------------------------------------------------------
void Display::enableDisplay(bool startMultiplexing)
{
    heatwire.write(true);
    boostConverter.write(true);

    if (startMultiplexing)
        dimming.initPwm(); // this line starts multiplexing by enabling the timer interrupt
                           // the rest of the multiplexing is done inside timer interrupt
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
    sendSegmentBits(bits, true, true, true, true);
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
void Display::sendSegmentBits(uint32_t bits, bool forceLatch, bool enableDots, bool enableUpperBar,
                              bool enableLowerBar)
{
    bits <<= 3; // shift bits to make place for N (upperBar), O_DP (dots) and P_MIN_SEC (lowerBar)
    bits |= ((enableUpperBar & 1) << 2) | ((enableDots & 1) << 1) | (enableLowerBar & 1);

    for (auto i = 0; i < NumberBitsInShiftRegister; i++)
    {
        shiftRegisterData.write((bits >> i) & 1);
        clockPeriod();
    }

    if (forceLatch)
        strobePeriod();
}

//-----------------------------------------------------------------
void Display::multiplexingInterrupt()
{
    if (++gridIndex >= NumberOfGrids)
        gridIndex = 0;

    sendSegmentBits(gridDataArray[gridIndex].segments, false, gridDataArray[gridIndex].enableDots,
                    gridDataArray[gridIndex].enableUpperBar,
                    gridDataArray[gridIndex].enableLowerBar);
    disableAllGrids();
    strobePeriod();

    gridGpioArray[gridIndex].write(true);
}

//--------------------------------------------------------------------------------------------------
inline void Display::disableAllGrids()
{
    for (auto &grid : gridGpioArray)
        grid.write(false);
}

//--------------------------------------------------------------------------------------------------
void Display::setClock(Time clockToShow)
{
    currentTime = clockToShow;
}

//--------------------------------------------------------------------------------------------------
void Display::showClock(bool forceShowDots)
{
    // add '0' to get the ASCII value of the number
    gridDataArray[1].segments = font.getGlyph((currentTime.hour / 10) + '0');
    gridDataArray[2].segments = font.getGlyph((currentTime.hour % 10) + '0');
    gridDataArray[3].segments = font.getGlyph((currentTime.minute / 10) + '0');
    gridDataArray[4].segments = font.getGlyph((currentTime.minute % 10) + '0');

    gridDataArray[2].enableDots = (currentTime.second % 2) == 0 || forceShowDots;
}