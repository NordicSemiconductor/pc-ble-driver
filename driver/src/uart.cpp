/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

#include "uart.h"

Uart::Uart(data_received_t dataReceived, data_sent_t dataSent)
    : dataReceivedHandle(dataReceived),
      dataSentHandle(dataSent)
{
}

Uart::~Uart()
{
}

void Uart::registerCallbacks(data_received_t dataReceived, data_sent_t dataSent)
{
    this->dataReceivedHandle = dataReceived;
    this->dataSentHandle = dataSent;
}
