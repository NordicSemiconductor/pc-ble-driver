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

#ifndef UART_SETTINGS_H
#define UART_SETTINGS_H

#include <stdint.h>
#include <string>

/*@brief Flow control modes. */
typedef enum
{
    UartFlowControlNone,
    UartFlowControlSoftware,
    UartFlowControlHardware
} UartFlowControl;

/*@brief Parity modes. */
typedef enum
{
    UartParityNone,
    UartParityOdd,
    UartParityEven
} UartParity;

/*@brief Stop bit modes. */
typedef enum
{
    UartStopBitsOne,
    UartStopBitsOnePointFive,
    UartStopBitsTwo
} UartStopBits;

/*@brief Data bit modes. */
typedef enum
{
    UartDataBitsFive  = 5,
    UartDataBitsSix   = 6,
    UartDataBitsSeven = 7,
    UartDataBitsEight = 8
} UartDataBits;

/*@brief UART communication parameters. */
typedef struct
{
    const char * portName;
    uint32_t baudRate;
    UartFlowControl flowControl;
    UartParity parity;
    UartStopBits stopBits;
    UartDataBits dataBits;
} UartCommunicationParameters;

/**
 * @brief The UartSettings class parses a uart_comm_params_t struct and gives out corresponding
 * boost enum values for use in the UartBoost class.
 */
class UartSettings
{
public:

    /*@brief Constructor. */
    UartSettings();

    /*@brief Constructor that initializes with communication paramters. */
    UartSettings(const UartCommunicationParameters &communicationParameters);

    /*@brief Destructor. */
    virtual ~UartSettings();

    /*@brief Sets the name of the serial port. */
    void setPortName(const std::string value);

    /*@brief Sets the baud rate. */
    void setBaudRate(const uint32_t value);

    /*@brief Sets the flow control parameter. */
    void setFlowControl(const UartFlowControl value);

    /*@brief Sets the Parity parameter. */
    void setParity(const UartParity value);

    /*@brief Sets the Stop Bits parameter. */
    void setStopBits(const UartStopBits value);

    /*@brief Sets the Data Bits parameter. */
    void setDataBits(const UartDataBits value);

    /*@brief Returns the currently configured serial port name. */
    std::string getPortName();

    /*@brief Returns the currently configured Baud Rate. */
    uint32_t getBaudRate() const;

    /*@brief Returns the currently configured Flow Control setting. */
    UartFlowControl getFlowControl() const;

    /*@brief Returns the currently configured Parity setting. */
    UartParity getParity() const;

    /*@brief Returns the currently configured Stop Bits setting. */
    UartStopBits getStopBits() const;

    /*@brief Returns the currently configured Data Bits setting. */
    UartDataBits getDataBits() const;

protected:

    std::string portName;
    uint32_t baudRate;
    UartFlowControl flowControl;
    UartParity parity;
    UartStopBits stopBits;
    UartDataBits dataBits;

};

#endif //UART_SETTINGS_H
