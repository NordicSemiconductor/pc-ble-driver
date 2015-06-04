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

#ifndef MOCK_UARTBOOST_H
#define MOCK_UARTBOOST_H

#include "uart.h"
#include "uart_defines.h"

class UartBoost : public Uart
{
public:
    UartBoost(const data_received_t dataReceived, const data_sent_t dataSent);

    uint32_t open(const UartCommunicationParameters &communicationParameters);
    bool isOpen() const;
    uint32_t close();
    uint32_t write(const uint8_t *data, uint16_t length);
    uint32_t write(const uint8_t data);
    uint32_t flush();

    void receiveDataEvent(uint32_t error_code, uint8_t *data, uint16_t length);
    void sendDataEvent(uint32_t error_code, uint16_t length);
    static void reset();

    static UartCommunicationParameters comParameters;
    static bool shouldOpenNormally;
    static uint32_t returnOnOpen;
    static bool shouldCloseNormally;
    static uint32_t returnOnClose;
    static bool opened;
    static char dataWritten;
    static UartBoost *instance;
};

#endif // MOCK_UARTBOOST_H
