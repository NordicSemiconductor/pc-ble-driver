/*
 * Copyright (c) 2016 Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 *   3. Neither the name of Nordic Semiconductor ASA nor the names of other
 *   contributors to this software may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *   4. This software must only be used in or with a processor manufactured by Nordic
 *   Semiconductor ASA, or in or with a processor manufactured by a third party that
 *   is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 *   5. Any software provided in binary or object form under this license must not be
 *   reverse engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
