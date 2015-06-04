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

#ifndef TST_APP_LOG_H
#define TST_APP_LOG_H

#include "uart.h"
#include "uart_defines.h"
#include "app_uart_extension.h"
#include "app_uart.h"

#include <vector>

using std::vector;

class AppUartTest
{

public:
    AppUartTest();
    ~AppUartTest();

    static AppUartTest *instance();

    void getCommunicationParameters();
    UartCommunicationParameters comParameters;

    void addEvent(app_uart_evt_t *inEvent);
    void clearEvents();
    void confirmEvent(app_uart_evt_type_t eventType, uint8_t *data, uint32_t errorCode, uint16_t length);

private:
    vector<app_uart_evt_t> events;
    static AppUartTest *_instance;
};

// These functions are implemented in main.cpp
void genericEventHandler(app_uart_evt_t *p_app_uart_event);
void loggingEventHandler(app_uart_evt_t *p_app_uart_event);

#endif // TST_APP_LOG_H
