#pragma once

#include "I2cAccessor.hpp"
#include "Time/Time.hpp"

#include <optional>
#include <stdint.h>

namespace ds3231
{
constexpr I2cAccessor::DeviceAddress SlaveAddress = 0x68; // 7-bit slave address

enum class Register : uint8_t
{
    Seconds = 0x0,
    Minutes = 0x1,
    Hour = 0x2,
    DayOfWeek = 0x3,
    Date = 0x4,
    Month_Century = 0x5,
    Year = 0x6,
    Alarm1_Seconds = 0x7,
    Alarm1_Minutes = 0x8,
    Alarm1_Hours = 0x9,
    Alarm1_DayofWeekDate = 0xA,
    Alarm2_Minutes = 0xB,
    Alarm2_Hours = 0xC,
    Alarm2_DayofWeekDate = 0xD,
    Control = 0xE,
    Control_Status = 0xF,
    AgingOffset = 0x10,
    MSBTemperature = 0x11,
    LSBTemperature = 0x12
};

constexpr auto AlarmMaskBit = 7;
constexpr auto Format12_24Bit = 6;

namespace control_bits
{
constexpr auto EOSC = 7;  // oscillator enable
constexpr auto BBSQW = 6; // battery-backed square-wave enable
constexpr auto Conv = 5;  // convert temperature

constexpr auto RateSelectMask = 0b11; // rate select oscillator frequency
constexpr auto RateSelectPos = 3;

constexpr auto INTCN = 2; // interrupt control
constexpr auto A2IE = 1;  // alarm2 interrupt enable
constexpr auto A1IE = 0;  // alarm1 interrupt enable
} // namespace control_bits

namespace status_bits
{
constexpr auto OSF = 7;     // oscillator stop flag
constexpr auto EN32kHz = 3; // controls the status of 32kHz output pin
constexpr auto Busy = 2;    // busy flag
constexpr auto A2F = 1;     // alarm2 flag
constexpr auto A1F = 0;     // alarm 1 flag

} // namespace status_bits

enum class SqwRate : uint8_t
{
    Freq1Hz = 0,
    Freq1024Hz = 1,
    Freq4096Hz = 2,
    Freq8192Hz = 3
};

} // namespace ds3231

struct Date
{
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t dow;
};

class DS3231
{
public:
    DS3231(I2cAccessor &accessor) : accessor(accessor){};

    [[nodiscard]] std::optional<Time> getTime();
    [[nodiscard]] std::optional<Date> getDate();
    void setTime(uint8_t hour, uint8_t min, uint8_t sec);
    void setHour(uint8_t hour);
    void setDate(uint8_t day, uint8_t month, uint16_t year);
    void setDOW(uint8_t dow);

    void setAlarm1(const uint8_t sec, const uint8_t min, const uint8_t hour);
    [[nodiscard]] std::optional<Time> getAlarm1();
    void clearAlarm1Flag();
    [[nodiscard]] std::optional<bool> isAlarm1Triggered();
    void setAlarm1Interrupt(bool enable);

    void setAlarm2(const uint8_t min, const uint8_t hour);
    [[nodiscard]] std::optional<Time> getAlarm2();
    void clearAlarm2Flag();
    [[nodiscard]] std::optional<bool> isAlarm2Triggered();
    void setAlarm2Interrupt(bool enable);

    void forceTemperatureUpdate();
    std::optional<float> getTemperature();

    void enable32KHz(bool enable);
    void setInterruptOutput(bool enable);
    void setSQWRate(ds3231::SqwRate rate);

    bool isCommunicationFailed()
    {
        return accessor.hasError();
    }

private:
    I2cAccessor &accessor;

    enum class Alarm
    {
        Alarm1,
        Alarm2
    };

    void clearAlarmFlag(Alarm alarm);
    std::optional<bool> isAlarmTriggered(Alarm alarm);
    void setAlarmInterrupt(bool enable, Alarm alarm);

    [[nodiscard]] uint8_t decToBcd(uint8_t val);
    [[nodiscard]] uint8_t bcdToDec(uint8_t val);

    bool updateRegister(ds3231::Register registerName, uint8_t bitPos, bool bitState);
};
