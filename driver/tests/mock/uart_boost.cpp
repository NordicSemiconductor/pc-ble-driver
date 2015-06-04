/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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

#include "uart_boost.h"
#include "nrf_error.h"

UartCommunicationParameters UartBoost::comParameters;
bool UartBoost::shouldOpenNormally = true;
uint32_t UartBoost::returnOnOpen = NRF_SUCCESS;
bool UartBoost::shouldCloseNormally = true;
uint32_t UartBoost::returnOnClose = NRF_SUCCESS;
bool UartBoost::opened = false;
char UartBoost::dataWritten = ' ';
UartBoost *UartBoost::instance = NULL;

UartBoost::UartBoost(const data_received_t dataReceived, const data_sent_t dataSent)
    : Uart(dataReceived, dataSent)
{
}

uint32_t UartBoost::open(const UartCommunicationParameters &communicationParameters)
{
    instance = this;
    comParameters = communicationParameters;

    if (shouldOpenNormally)
    {
        opened = true;
    }

    return returnOnOpen;
}

bool UartBoost::isOpen() const
{
    return opened;
}

uint32_t UartBoost::close()
{
    instance = NULL;

    if (shouldCloseNormally)
    {
        opened = false;
    }

    return returnOnClose;
}

uint32_t UartBoost::write(const uint8_t *data, uint16_t length)
{
    (void)length;
    dataWritten = data[0];
    return NRF_SUCCESS;
}

uint32_t UartBoost::write(const uint8_t data)
{
    dataWritten = data;
    return NRF_SUCCESS;
}

uint32_t UartBoost::flush()
{
    return NRF_SUCCESS;
}

void UartBoost::reset()
{
    shouldOpenNormally = true;
    returnOnOpen = NRF_SUCCESS;
    shouldCloseNormally = true;
    returnOnClose = NRF_SUCCESS;
    opened = false;
    dataWritten = ' ';
}

void UartBoost::receiveDataEvent(uint32_t error_code, uint8_t *data, uint16_t length)
{
    dataReceivedHandle(error_code, data, length);
}

void UartBoost::sendDataEvent(uint32_t error_code, uint16_t length)
{
    dataSentHandle(error_code, length);
}
