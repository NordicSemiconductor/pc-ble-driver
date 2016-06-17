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

#include "uart_settings.h"

UartSettings::UartSettings()
    : portName("COM1"),
      baudRate(0),
      flowControl(UartFlowControlNone),
      parity(UartParityNone),
      stopBits(UartStopBitsOne),
      dataBits(UartDataBitsEight)
{
}

UartSettings::UartSettings(const UartCommunicationParameters &communicationParameters)
    : portName(communicationParameters.portName),
      baudRate(communicationParameters.baudRate),
      flowControl(communicationParameters.flowControl),
      parity(communicationParameters.parity),
      stopBits(communicationParameters.stopBits),
      dataBits(communicationParameters.dataBits)
{
}

UartSettings::~UartSettings()
{
}

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
