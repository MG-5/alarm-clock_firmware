#include "DS3231.hpp"

static const uint8_t dim[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

using namespace ds3231;

//--------------------------------------------------------------------------------------------------
std::optional<Time> DS3231::getTime()
{
    constexpr auto NumberOfBytes = 3;
    uint8_t data[NumberOfBytes]{0}; // Second, Minute, Hour
    accessor.beginTransaction(SlaveAddress);
    bool transactionResult = accessor.readFromRegister(Register::Seconds, data, NumberOfBytes);
    accessor.endTransaction();

    if (transactionResult)
        return Time(bcdToDec(data[2]), bcdToDec(data[1]), bcdToDec(data[0]));

    return {};
}

//--------------------------------------------------------------------------------------------------
std::optional<Date> DS3231::getDate()
{
    constexpr auto NumberOfBytes = 4;
    uint8_t data[NumberOfBytes]{0}; // DOW, Day, Month, Year
    accessor.beginTransaction(SlaveAddress);
    bool transactionResult = accessor.readFromRegister(Register::DayOfWeek, data, NumberOfBytes);
    accessor.endTransaction();

    if (transactionResult)
    {
        Date date;
        date.dow = bcdToDec(data[0]);
        date.day = bcdToDec(data[1]);
        date.month = bcdToDec(data[2]);
        date.year = bcdToDec(data[3]) + 2000;
        return date;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
void DS3231::setTime(uint8_t hour, uint8_t min, uint8_t sec)
{
    if (hour >= 24 || min >= 60 || sec >= 60)
        return;

    constexpr auto NumberOfBytes = 3;
    uint8_t dataToWrite[NumberOfBytes] = {decToBcd(sec), decToBcd(min), decToBcd(hour)};

    accessor.beginTransaction(SlaveAddress);
    accessor.writeToRegister(Register::Seconds, dataToWrite, NumberOfBytes);
    accessor.endTransaction();
}

//--------------------------------------------------------------------------------------------------
void DS3231::setHour(uint8_t hour)
{
    if (hour >= 24)
        return;

    accessor.beginTransaction(SlaveAddress);
    accessor.writeByteToRegister(Register::Hour, decToBcd(hour));
    accessor.endTransaction();
}

//--------------------------------------------------------------------------------------------------
void DS3231::setDate(uint8_t day, uint8_t month, uint16_t year)
{
    bool areParametersInRange = ((day >= 1) && (day <= 31)) && ((month >= 1) && (month <= 12)) &&
                                ((year >= 2000) && (year < 2100));

    if (!areParametersInRange)
        return;

    constexpr auto NumberOfBytes = 3;
    uint8_t dataToWrite[NumberOfBytes] = {decToBcd(day), decToBcd(month), decToBcd(year - 2000)};

    accessor.beginTransaction(SlaveAddress);
    accessor.writeToRegister(Register::Date, dataToWrite, NumberOfBytes);
    accessor.endTransaction();
}

//--------------------------------------------------------------------------------------------------
void DS3231::setDOW(uint8_t dow)
{
    if (dow >= 8)
        return;

    accessor.beginTransaction(SlaveAddress);
    accessor.writeByteToRegister(Register::DayOfWeek, dow);
    accessor.endTransaction();
}

//--------------------------------------------------------------------------------------------------
void DS3231::setAlarm1(const uint8_t sec, const uint8_t min, const uint8_t hour)
{
    if (sec >= 60 || min >= 60 || hour >= 24)
        return;

    constexpr auto NumberOfBytes = 4;
    uint8_t dataToWrite[NumberOfBytes] = {
        decToBcd(sec),     //
        decToBcd(min),     //
        decToBcd(hour),    //
        1 << AlarmMaskBit, // enable A1M4 bit
    };

    accessor.beginTransaction(SlaveAddress);
    accessor.writeToRegister(Register::Alarm1_Seconds, dataToWrite, NumberOfBytes);
    accessor.endTransaction();
}

//--------------------------------------------------------------------------------------------------
std::optional<Time> DS3231::getAlarm1()
{
    constexpr auto NumberOfBytes = 3;
    uint8_t data[NumberOfBytes]{0}; // second, minute, hour

    accessor.beginTransaction(SlaveAddress);
    bool transactionResult =
        accessor.readFromRegister(Register::Alarm1_Seconds, data, NumberOfBytes);
    accessor.endTransaction();

    if (transactionResult)
        return Time(bcdToDec(data[2]), bcdToDec(data[1]), bcdToDec(data[0]));

    return {};
}

//--------------------------------------------------------------------------------------------------
std::optional<bool> DS3231::isAlarm1Triggered()
{
    return isAlarmTriggered(Alarm::Alarm1);
}

//--------------------------------------------------------------------------------------------------
void DS3231::clearAlarm1Flag()
{
    clearAlarmFlag(Alarm::Alarm1);
}

//--------------------------------------------------------------------------------------------------
void DS3231::setAlarm1Interrupt(bool enable)
{
    setAlarmInterrupt(enable, Alarm::Alarm1);
}

//--------------------------------------------------------------------------------------------------
void DS3231::setAlarm2(const uint8_t min, const uint8_t hour)
{
    if (min >= 60 || hour >= 24)
        return;

    constexpr auto NumberOfBytes = 3;
    uint8_t dataToWrite[NumberOfBytes] = {
        decToBcd(min),     //
        decToBcd(hour),    //
        1 << AlarmMaskBit, // enable A1M4 bit
    };

    accessor.beginTransaction(SlaveAddress);
    accessor.writeToRegister(Register::Alarm2_Minutes, dataToWrite, NumberOfBytes);
    accessor.endTransaction();
}

//--------------------------------------------------------------------------------------------------
std::optional<Time> DS3231::getAlarm2()
{
    constexpr auto NumberOfBytes = 2;
    uint8_t data[NumberOfBytes]{0}; // minute, hour

    accessor.beginTransaction(SlaveAddress);
    bool transactionResult =
        accessor.readFromRegister(Register::Alarm2_Minutes, data, NumberOfBytes);
    accessor.endTransaction();

    if (transactionResult)
        return Time(bcdToDec(data[2]), bcdToDec(data[1]));

    return {};
}

//--------------------------------------------------------------------------------------------------
std::optional<bool> DS3231::isAlarm2Triggered()
{
    return isAlarmTriggered(Alarm::Alarm2);
}

//--------------------------------------------------------------------------------------------------
void DS3231::clearAlarm2Flag()
{
    clearAlarmFlag(Alarm::Alarm2);
}

//--------------------------------------------------------------------------------------------------
void DS3231::setAlarm2Interrupt(bool enable)
{
    setAlarmInterrupt(enable, Alarm::Alarm2);
}

//--------------------------------------------------------------------------------------------------
std::optional<bool> DS3231::isAlarmTriggered(Alarm alarm)
{
    uint8_t statusByte = 0;

    accessor.beginTransaction(SlaveAddress);
    bool transactionResult = accessor.readByteFromRegister(Register::Control_Status, statusByte);

    if (transactionResult)
    {
        const auto BitPosition = alarm == Alarm::Alarm1 ? status_bits::A1F : status_bits::A2F;
        return (statusByte >> BitPosition) & 1;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
void DS3231::clearAlarmFlag(Alarm alarm)
{
    const auto BitPosition = alarm == Alarm::Alarm1 ? status_bits::A1F : status_bits::A2F;
    updateRegister(Register::Control_Status, BitPosition, false);
}

//--------------------------------------------------------------------------------------------------
void DS3231::setAlarmInterrupt(bool enable, Alarm alarm)
{
    const auto BitPosition = alarm == Alarm::Alarm1 ? control_bits::A1IE : control_bits::A2IE;
    updateRegister(Register::Control, BitPosition, enable);
}

//--------------------------------------------------------------------------------------------------
void DS3231::forceTemperatureUpdate()
{
    updateRegister(Register::Control, control_bits::Conv, true);
}

//--------------------------------------------------------------------------------------------------
std::optional<float> DS3231::getTemperature()
{
    constexpr auto NumberOfBytes = 2;
    uint8_t data[NumberOfBytes];

    accessor.beginTransaction(SlaveAddress);
    bool transactionResult =
        accessor.readFromRegister(Register::MSBTemperature, data, NumberOfBytes);
    accessor.endTransaction();

    if (!transactionResult)
        return {};

    int8_t decimalPart = data[0];
    uint8_t fractionPart = data[1] >> 6;

    return decimalPart + (fractionPart * 0.25f);
}

//--------------------------------------------------------------------------------------------------
void DS3231::enable32KHz(bool enable)
{
    updateRegister(Register::Control_Status, status_bits::EN32kHz, enable);
}

//--------------------------------------------------------------------------------------------------
void DS3231::setInterruptOutput(bool enable)
{
    updateRegister(Register::Control, control_bits::INTCN, enable);
}

//--------------------------------------------------------------------------------------------------
void DS3231::setSQWRate(SqwRate rate)
{
    uint8_t controlByte = 0;

    accessor.beginTransaction(SlaveAddress);
    bool transactionResult = accessor.readByteFromRegister(Register::Control, controlByte);

    if (!transactionResult)
    {
        accessor.endTransaction();
        return;
    }

    controlByte &= ~(control_bits::RateSelectMask << control_bits::RateSelectPos);
    controlByte |= (static_cast<uint8_t>(rate) << control_bits::RateSelectPos);
    accessor.writeByteToRegister(Register::Control, controlByte);
    accessor.endTransaction();
}

//--------------------------------------------------------------------------------------------------
uint8_t DS3231::decToBcd(uint8_t val)
{
    return ((val / 10) << 4) + (val % 10);
}

//--------------------------------------------------------------------------------------------------
uint8_t DS3231::bcdToDec(uint8_t val)
{
    return ((val >> 4) * 10) + (val & 0b1111);
}

//--------------------------------------------------------------------------------------------------
bool DS3231::updateRegister(Register registerName, uint8_t bitPos, bool bitState)
{
    uint8_t registerContent = 0;

    accessor.beginTransaction(SlaveAddress);
    bool transactionResult = accessor.readByteFromRegister(registerName, registerContent);

    if (!transactionResult)
    {
        accessor.endTransaction();
        return false;
    }

    bitState ? registerContent |= (1 << bitPos) : registerContent &= ~(1 << bitPos);
    transactionResult = accessor.writeByteToRegister(registerName, registerContent);
    accessor.endTransaction();

    return transactionResult;
}
