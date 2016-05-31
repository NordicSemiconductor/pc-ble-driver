/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "uart_settings_boost.h"

#include <iostream>

using namespace boost;

UartSettingsBoost::UartSettingsBoost()
    : UartSettings()
{
    //Empty
}

UartSettingsBoost::UartSettingsBoost(const UartCommunicationParameters &communicationParameters)
    : UartSettings(communicationParameters)
{
    //Empty
}

asio::serial_port::baud_rate UartSettingsBoost::getBoostBaudRate() const
{
    return asio::serial_port::baud_rate(baudRate);
}

asio::serial_port::flow_control UartSettingsBoost::getBoostFlowControl() const
{
    switch (flowControl)
    {
        case UartFlowControlNone:
            return asio::serial_port::flow_control(asio::serial_port::flow_control::none);
        case UartFlowControlSoftware:
            return asio::serial_port::flow_control(asio::serial_port::flow_control::software);
        case UartFlowControlHardware:
            return asio::serial_port::flow_control(asio::serial_port::flow_control::hardware);
        default:
            std::cerr << "Invalid flowcontrol setting " << flowControl <<", defaulting to flow_control::none!";
            return asio::serial_port::flow_control(asio::serial_port::flow_control::none);
    }
}

asio::serial_port::parity UartSettingsBoost::getBoostParity() const
{
    switch (parity)
    {
        case UartParityNone:
            return asio::serial_port::parity(asio::serial_port::parity::none);
        case UartParityOdd:
            return asio::serial_port::parity(asio::serial_port::parity::odd);
        case UartParityEven:
            return asio::serial_port::parity(asio::serial_port::parity::even);
        default:
            std::cerr << "Invalid parity setting " << parity <<", defaulting to parity::none!";
            return asio::serial_port::parity(asio::serial_port::parity::none);
    }
}

asio::serial_port::stop_bits UartSettingsBoost::getBoostStopBits() const
{
    switch (stopBits)
    {
        case UartStopBitsOne:
            return asio::serial_port::stop_bits(asio::serial_port::stop_bits::one);
        case UartStopBitsOnePointFive:
            return asio::serial_port::stop_bits(asio::serial_port::stop_bits::onepointfive);
        case UartStopBitsTwo:
            return asio::serial_port::stop_bits(asio::serial_port::stop_bits::two);
        default:
            std::cerr << "Invalid stopbits setting " << stopBits <<", defaulting to stop_bits::one!";
            return asio::serial_port::stop_bits(asio::serial_port::stop_bits::one);
    }
}

asio::serial_port::character_size UartSettingsBoost::getBoostCharacterSize() const
{
    return asio::serial_port::character_size(dataBits);
}
