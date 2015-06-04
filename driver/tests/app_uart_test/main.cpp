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
#include "app_log.h"
#include "uart_boost.h"
#include "uart.h"
#include "uart_defines.h"
#include "app_uart_extension.h"
#include "app_uart.h"

#include <map>
#include <string>

using std::map;
using std::string;

#define BOOST_TEST_MODULE app_uart_test
#include <boost/test/unit_test.hpp>

// Suite fixture setup
struct SuiteFixture {
    SuiteFixture() {
        app_log_file_path_set("./log.log");
        UartBoost::reset();
        AppUartTest::instance(); // Create an instance
    }

    ~SuiteFixture() {
        UartBoost::shouldCloseNormally = true;
        app_uart_close(0);
        delete AppUartTest::instance();
    }
};

void genericEventHandler(app_uart_evt_t *p_app_uart_event);
void loggingEventHandler(app_uart_evt_t *p_app_uart_event);

BOOST_FIXTURE_TEST_SUITE( app_uart_test, SuiteFixture )

BOOST_AUTO_TEST_CASE( testPortNameSet )
{
    map<string, int> portNames;

    portNames["COM5"] = NRF_SUCCESS;
    portNames["COM99"] = NRF_SUCCESS;
    portNames["COM1000"] = NRF_SUCCESS;
    portNames["/dev/ttyS0"] = NRF_SUCCESS;

    for(map<string, int>::iterator portName = portNames.begin(); portName != portNames.end(); ++portName)
    {
        BOOST_CHECK_EQUAL(app_uart_port_name_set((*portName).first.c_str()), (*portName).second);
        AppUartTest::instance()->getCommunicationParameters();
        BOOST_CHECK_EQUAL(AppUartTest::instance()->comParameters.portName, (*portName).first);
    }
}

BOOST_AUTO_TEST_CASE( testBaudRateSet )
{
    AppUartTest *aut = AppUartTest::instance();

    map<int, int> baudRates;
    baudRates[1200] = NRF_SUCCESS;
    baudRates[2400] = NRF_SUCCESS;
    baudRates[4800] = NRF_SUCCESS;
    baudRates[9600] = NRF_SUCCESS;
    baudRates[14400] = NRF_SUCCESS;
    baudRates[28800] = NRF_SUCCESS;
    baudRates[38400] = NRF_SUCCESS;
    baudRates[57600] = NRF_SUCCESS;
    baudRates[76800] = NRF_SUCCESS;
    baudRates[115200] = NRF_SUCCESS;
    baudRates[230400] = NRF_SUCCESS;
    baudRates[250000] = NRF_SUCCESS;
    baudRates[460800] = NRF_SUCCESS;
    baudRates[921600] = NRF_SUCCESS;
    baudRates[1000000] = NRF_SUCCESS;

    baudRates[0] = NRF_ERROR_INVALID_PARAM;
    baudRates[1199] = NRF_ERROR_INVALID_PARAM;
    baudRates[1202] = NRF_ERROR_INVALID_PARAM;
    baudRates[1000001] = NRF_ERROR_INVALID_PARAM;

    for(map<int, int>::iterator baudRate = baudRates.begin();
        baudRate != baudRates.end();
        ++baudRate)
    {
        BOOST_CHECK_EQUAL((*baudRate).second, app_uart_baud_rate_set((*baudRate).first));

        // Only check baud rates when the expected result is NRF_SUCCESS
        if((*baudRate).second == NRF_SUCCESS)
        {
            aut->getCommunicationParameters();
            BOOST_CHECK_EQUAL((*baudRate).first, aut->comParameters.baudRate);
        }

        app_uart_close(0);
    }
}

BOOST_AUTO_TEST_CASE( testFlowControlSet )
{
    AppUartTest *aut = AppUartTest::instance();

    map<app_uart_extension_flow_control_t, UartFlowControl> flowControlsSuccess;
    flowControlsSuccess[APP_UART_FLOW_CONTROL_NONE] = UartFlowControlNone;
    flowControlsSuccess[APP_UART_FLOW_CONTROL_HARDWARE] = UartFlowControlHardware;

    for(map<app_uart_extension_flow_control_t, UartFlowControl>::iterator ii=flowControlsSuccess.begin();
        ii!=flowControlsSuccess.end();
        ++ii)
    {
        app_uart_extension_flow_control_t flow_control = (*ii).first;
        UartFlowControl expectedFlowControl = (*ii).second;
        BOOST_CHECK_EQUAL(NRF_SUCCESS, app_uart_flow_control_set(flow_control));
        aut->getCommunicationParameters();
        BOOST_CHECK_EQUAL(aut->comParameters.flowControl, expectedFlowControl);
        app_uart_close(0);
    }

    map<int, int> flowControlsFail;
    flowControlsFail[APP_UART_FLOW_CONTROL_NONE - 1] = (int)UartFlowControlNone;
    flowControlsFail[APP_UART_FLOW_CONTROL_HARDWARE + 1] = (int)UartFlowControlNone;

    for(map<int, int>::iterator ii=flowControlsFail.begin(); ii!=flowControlsFail.end(); ++ii)
    {
        app_uart_extension_flow_control_t flow_control = (app_uart_extension_flow_control_t)(*ii).first;
        BOOST_CHECK_EQUAL(NRF_ERROR_INVALID_PARAM, app_uart_flow_control_set(flow_control));
        app_uart_close(0);
    }
}

BOOST_AUTO_TEST_CASE( testParitySet )
{
    AppUartTest *aut = AppUartTest::instance();
    map<app_uart_extension_parity_t, UartParity> paritySuccess;
    paritySuccess[APP_UART_PARITY_NONE] = UartParityNone;
    paritySuccess[APP_UART_PARITY_ODD] = UartParityOdd;
    paritySuccess[APP_UART_PARITY_EVEN] = UartParityEven;

    app_uart_close(0);

    for(map<app_uart_extension_parity_t, UartParity>::iterator ii=paritySuccess.begin();
        ii!=paritySuccess.end();
        ++ii)
    {
        app_uart_extension_parity_t parity = (*ii).first;
        UartParity expectedParity = (*ii).second;
        BOOST_CHECK_EQUAL(NRF_SUCCESS, app_uart_parity_set(parity));
        aut->getCommunicationParameters();
        BOOST_CHECK_EQUAL(expectedParity, aut->comParameters.parity);
        app_uart_close(0);
    }

    map<int, int> parityFailure;
    parityFailure[APP_UART_PARITY_NONE - 1] = (int)UartParityNone;
    parityFailure[APP_UART_PARITY_EVEN + 1] = (int)UartParityNone;

    for( map<int, int>::iterator ii = parityFailure.begin(); ii != parityFailure.end(); ++ii)
    {
        app_uart_extension_parity_t parity = (app_uart_extension_parity_t)(*ii).first;
        BOOST_CHECK_EQUAL(NRF_ERROR_INVALID_PARAM, app_uart_parity_set(parity));
        app_uart_close(0);
    }
}

BOOST_AUTO_TEST_CASE( testAppUartInit )
{
    AppUartTest *aut = AppUartTest::instance();

    app_uart_close(0);

    BOOST_CHECK_EQUAL((uint32_t)NRF_SUCCESS, app_uart_init(0, 0, &genericEventHandler, APP_IRQ_PRIORITY_HIGH, 0));
    BOOST_CHECK_EQUAL((uint32_t)NRF_ERROR_INVALID_STATE, app_uart_init(0, 0, &genericEventHandler, APP_IRQ_PRIORITY_HIGH, 0));

    app_uart_close(0);

    BOOST_CHECK_EQUAL((uint32_t)NRF_ERROR_NULL, app_uart_init(0, 0, 0 /*eventhandler*/, APP_IRQ_PRIORITY_HIGH, 0));
    app_uart_close(0);

    app_uart_port_name_set("");
    BOOST_CHECK_EQUAL((uint32_t)NRF_SUCCESS, app_uart_init(0, 0, &genericEventHandler, APP_IRQ_PRIORITY_HIGH, 0));
    aut->getCommunicationParameters();
    BOOST_CHECK_EQUAL("COM1", aut->comParameters.portName);

    app_uart_close(0);

    UartBoost::shouldOpenNormally = false;
    BOOST_CHECK_EQUAL((uint32_t)NRF_ERROR_INVALID_STATE, app_uart_init(0, 0, &genericEventHandler, APP_IRQ_PRIORITY_HIGH, 0));

    app_uart_close(0);
}

BOOST_AUTO_TEST_CASE( testAppUartGet )
{
    BOOST_CHECK_EQUAL((uint32_t)NRF_ERROR_NOT_SUPPORTED, app_uart_get(0));
}

BOOST_AUTO_TEST_CASE( testAppUartPut )
{
    BOOST_CHECK_EQUAL((uint32_t)NRF_ERROR_INVALID_STATE, app_uart_put('0'));
    app_uart_init(0, 0, &genericEventHandler, APP_IRQ_PRIORITY_HIGH, 0);
    BOOST_CHECK_EQUAL((uint32_t)NRF_SUCCESS, app_uart_put('1'));
    BOOST_CHECK_EQUAL('1', UartBoost::dataWritten);
}


BOOST_AUTO_TEST_CASE( testAppUartGetConnectionState )
{
    app_uart_connection_state_t state;

    BOOST_CHECK_EQUAL((uint32_t)NRF_ERROR_INVALID_STATE, app_uart_put('0'));

    BOOST_CHECK_EQUAL((uint32_t)NRF_SUCCESS, app_uart_get_connection_state(&state));
    BOOST_CHECK_EQUAL(APP_UART_DISCONNECTED, state);

    app_uart_init(0, 0, &genericEventHandler, APP_IRQ_PRIORITY_HIGH, 0);

    BOOST_CHECK_EQUAL((uint32_t)NRF_SUCCESS, app_uart_get_connection_state(&state));
    BOOST_CHECK_EQUAL(APP_UART_CONNECTED, state);

    UartBoost::opened = false;

    BOOST_CHECK_EQUAL((uint32_t)NRF_SUCCESS, app_uart_get_connection_state(&state));
    BOOST_CHECK_EQUAL(APP_UART_DISCONNECTED, state);
}

BOOST_AUTO_TEST_CASE( testAppUartFlush )
{
    BOOST_CHECK_EQUAL((uint32_t)NRF_ERROR_INVALID_STATE, app_uart_flush());
    app_uart_init(0, 0, &genericEventHandler, APP_IRQ_PRIORITY_HIGH, 0);
    BOOST_CHECK_EQUAL((uint32_t)NRF_SUCCESS, app_uart_flush());
}

BOOST_AUTO_TEST_CASE( testAppUartClose )
{
    BOOST_CHECK_EQUAL((uint32_t)NRF_ERROR_INVALID_STATE, app_uart_close(0));
    app_uart_init(0, 0, &genericEventHandler, APP_IRQ_PRIORITY_HIGH, 0);
    BOOST_CHECK_EQUAL((uint32_t)NRF_SUCCESS, app_uart_close(0));
}

BOOST_AUTO_TEST_CASE( testDataReceivedEventHandler )
{
    AppUartTest *aut = AppUartTest::instance();
    app_uart_init(0, 0, &loggingEventHandler, APP_IRQ_PRIORITY_HIGH, 0);

    aut->clearEvents();
    UartBoost::instance->receiveDataEvent((uint32_t)NRF_SUCCESS, (uint8_t *)"SUCCESS", 7);
    aut->confirmEvent(APP_UART_DATA, (uint8_t *)"SUCCESS", 0, 7);
    aut->clearEvents();
    UartBoost::instance->receiveDataEvent((uint32_t)NRF_ERROR_INVALID_STATE, (uint8_t *)("FAILED"), 7);
    aut->confirmEvent(APP_UART_COMMUNICATION_ERROR, (uint8_t *)"FAILED", (uint32_t)NRF_ERROR_INVALID_STATE, 1);
}

BOOST_AUTO_TEST_CASE( testDataSentEventHandler )
{
    app_uart_init(0, 0, &loggingEventHandler, APP_IRQ_PRIORITY_HIGH, 0);

    UartBoost::instance->sendDataEvent((uint32_t)NRF_ERROR_INVALID_STATE, 1);
    AppUartTest::instance()->confirmEvent(APP_UART_COMMUNICATION_ERROR, NULL, NRF_ERROR_INVALID_STATE, 1);
}

BOOST_AUTO_TEST_SUITE_END()

void genericEventHandler(app_uart_evt_t *p_app_uart_event)
{
    do { (void)(p_app_uart_event); } while (0);
}

void loggingEventHandler(app_uart_evt_t *p_app_uart_event)
{
    AppUartTest::instance()->addEvent(p_app_uart_event);
}