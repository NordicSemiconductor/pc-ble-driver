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

#include "tst_app_uart.h"

#include "uart_boost.h"

#include <boost/test/unit_test.hpp>

AppUartTest *AppUartTest::_instance = NULL;

AppUartTest::AppUartTest()
    : comParameters(),
      events()
{
}

AppUartTest::~AppUartTest()
{
    AppUartTest::_instance = NULL;
}

AppUartTest *AppUartTest::instance()
{
    if(!_instance) {
        _instance = new AppUartTest();
    }

    return _instance;
}

void AppUartTest::addEvent(app_uart_evt_t *inEvent)
{
    app_uart_evt_t newEvent;
    newEvent.evt_type = inEvent->evt_type;

    if (newEvent.evt_type == APP_UART_DATA)
    {
        newEvent.data.value = inEvent->data.value;
    }
    else if (newEvent.evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        newEvent.data.error_communication = inEvent->data.error_communication;
    }
    else if (newEvent.evt_type == APP_UART_TX_EMPTY)
    {
        newEvent.data.error_code = inEvent->data.error_code;
    }

    events.push_back(newEvent);
}

void AppUartTest::getCommunicationParameters()
{
    app_uart_init(0, 0, &genericEventHandler, APP_IRQ_PRIORITY_HIGH, 0);
    comParameters = UartBoost::comParameters;
}

void AppUartTest::clearEvents()
{
    events.clear();
}

void AppUartTest::confirmEvent(app_uart_evt_type_t eventType, uint8_t *data, uint32_t errorCode, uint16_t length)
{
    BOOST_CHECK_EQUAL((int)length, events.size());
    int i = 0;

    for (std::vector<app_uart_evt_t>::iterator actualEvent = events.begin();
        actualEvent != events.end(); ++actualEvent)
    {
        BOOST_CHECK_EQUAL(eventType, actualEvent->evt_type);

        if (eventType == APP_UART_DATA)
        {
            BOOST_CHECK_EQUAL(data[i++], actualEvent->data.value);
        }
        else if (eventType == APP_UART_COMMUNICATION_ERROR)
        {
            BOOST_CHECK_EQUAL(errorCode, actualEvent->data.error_communication);
        }
        else if (eventType == APP_UART_TX_EMPTY)
        {
            BOOST_CHECK_EQUAL(errorCode, actualEvent->data.error_code);
        }
        else
        {
            BOOST_FAIL("UNKNOWN EVENT TYPE");
        }
    }
}
