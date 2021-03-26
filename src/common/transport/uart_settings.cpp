#include "uart_settings.h"

UartSettings::UartSettings()
    : portName("COM1")
    , baudRate(0)
    , flowControl(UartFlowControlNone)
    , parity(UartParityNone)
    , stopBits(UartStopBitsOne)
    , dataBits(UartDataBitsEight)
{}

UartSettings::UartSettings(const UartCommunicationParameters &communicationParameters)
    : portName(communicationParameters.portName)
    , baudRate(communicationParameters.baudRate)
    , flowControl(communicationParameters.flowControl)
    , parity(communicationParameters.parity)
    , stopBits(communicationParameters.stopBits)
    , dataBits(communicationParameters.dataBits)
{}

UartSettings::~UartSettings()
{}

void UartSettings::setPortName(const std::string value)
{
    portName = value;
}

void UartSettings::setBaudRate(const uint32_t value)
{
    baudRate = value;
}

void UartSettings::setFlowControl(const UartFlowControl value)
{
    flowControl = value;
}

void UartSettings::setParity(const UartParity value)
{
    parity = value;
}

void UartSettings::setStopBits(const UartStopBits value)
{
    stopBits = value;
}

void UartSettings::setDataBits(const UartDataBits value)
{
    dataBits = value;
}

std::string UartSettings::getPortName()
{
    return portName;
}

uint32_t UartSettings::getBaudRate() const
{
    return baudRate;
}

UartFlowControl UartSettings::getFlowControl() const
{
    return flowControl;
}

UartParity UartSettings::getParity() const
{
    return parity;
}

UartStopBits UartSettings::getStopBits() const
{
    return stopBits;
}

UartDataBits UartSettings::getDataBits() const
{
    return dataBits;
}
