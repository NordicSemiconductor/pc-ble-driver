/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

#include "app_uart.h"
#include "app_uart_extension.h"
#include "app_log.h"
#include "uart.h"
#include "uart_boost.h"

#include "nordic_common.h"  /* For UNUSED_PARAMETER(X) */
#include "nrf_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PORT_NAME   "COM1"

#define UART_BAUD_RATE_MAX_INDEX 16

static uint32_t m_baud_rates[] = {1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600,
                                  76800, 115200, 230400, 250000, 460800, 921600, 1000000};

static char                              m_uart_port_name[100] = {0};
static uint32_t                          m_baud_rate           = 115200;
static app_uart_extension_flow_control_t m_flow_control        = APP_UART_FLOW_CONTROL_NONE;
static app_uart_extension_parity_t       m_parity              = APP_UART_PARITY_NONE;

static app_uart_event_handler_t m_event_handler;                         /**< Event handler function. */
static Uart * mp_uart_object = NULL;


static void data_received(uint32_t error_code, uint8_t * data, uint16_t length)
{
    app_uart_evt_t app_uart_event;

    if (error_code != NRF_SUCCESS)
    {
        app_log_handler(APP_LOG_ERROR, "Error reading from UART, error code: 0x%02X.", error_code);

        app_uart_event.evt_type = APP_UART_COMMUNICATION_ERROR;
        app_uart_event.data.error_communication = error_code;
        m_event_handler(&app_uart_event);
        return;
    }

    app_uart_event.evt_type = APP_UART_DATA;
    for (int i = 0; i < length; i++)
    {
        app_uart_event.data.value = data[i];
        m_event_handler(&app_uart_event);
    }
}

static void data_sent(uint32_t error_code, uint16_t length)
{
    UNUSED_PARAMETER(length);
    app_uart_evt_t app_uart_event;

    if (error_code != NRF_SUCCESS)
    {
        app_log_handler(APP_LOG_ERROR, "Error writing to UART, error code: 0x%02X.", error_code);

        app_uart_event.evt_type = APP_UART_COMMUNICATION_ERROR;
        app_uart_event.data.error_communication = error_code;
        m_event_handler(&app_uart_event);
    }
}

static void fill_communications_struct(UartCommunicationParameters *p_uart_comm_params)
{
    p_uart_comm_params->portName = m_uart_port_name;
    p_uart_comm_params->baudRate = m_baud_rate;

    if (m_flow_control == APP_UART_FLOW_CONTROL_HARDWARE)
    {
        p_uart_comm_params->flowControl = UartFlowControlHardware;
    }
    else
    {
        p_uart_comm_params->flowControl = UartFlowControlNone;
    }

    if (m_parity == APP_UART_PARITY_EVEN)
    {
        p_uart_comm_params->parity = UartParityEven;
    }
    else if (m_parity == APP_UART_PARITY_ODD)
    {
        p_uart_comm_params->parity = UartParityOdd;
    }
    else
    {
        p_uart_comm_params->parity = UartParityNone;
    }

    p_uart_comm_params->stopBits = UartStopBitsOne;
    p_uart_comm_params->dataBits = UartDataBitsEight;
}

uint32_t app_uart_port_name_set(const char * port_name)
{
    strcpy(m_uart_port_name, port_name);
    return NRF_SUCCESS;
}

uint32_t app_uart_baud_rate_set(uint32_t baud_rate)
{
    bool valid_baud_rate = false;

    for (int i = 0; i < UART_BAUD_RATE_MAX_INDEX; i++)
    {
        valid_baud_rate |= (m_baud_rates[i] == baud_rate);
    }

    if (!valid_baud_rate)
    {
        app_log_handler(APP_LOG_ERROR, "%d is an invalid baud rate.", baud_rate);
        return NRF_ERROR_INVALID_PARAM;
    }

    m_baud_rate = baud_rate;
    return NRF_SUCCESS;
}

uint32_t app_uart_flow_control_set(app_uart_extension_flow_control_t flow_control)
{
    if (flow_control < APP_UART_FLOW_CONTROL_NONE ||
        APP_UART_FLOW_CONTROL_HARDWARE < flow_control)
    {
        app_log_handler (APP_LOG_ERROR, "%d is not a valid value for flow control.", flow_control);
        return NRF_ERROR_INVALID_PARAM;
    }

    m_flow_control = flow_control;
    return NRF_SUCCESS;
}

uint32_t app_uart_parity_set(app_uart_extension_parity_t parity)
{
    if (parity < APP_UART_PARITY_NONE || APP_UART_PARITY_EVEN < parity)
    {
        app_log_handler (APP_LOG_ERROR, "%d is not a valid value for parity.", parity);
        return NRF_ERROR_INVALID_PARAM;
    }

    m_parity = parity;
    return NRF_SUCCESS;
}

uint32_t app_uart_init(const app_uart_comm_params_t   * p_comm_params,
                             app_uart_buffers_t       * p_buffers,
                             app_uart_event_handler_t   event_handler,
                             app_irq_priority_t         irq_priority,
                             uint16_t                 * p_uart_uid)
{
    UNUSED_PARAMETER(p_comm_params);
    UNUSED_PARAMETER(p_buffers);
    UNUSED_PARAMETER(irq_priority);
    UNUSED_PARAMETER(p_uart_uid);

    if (event_handler == NULL)
    {
        return NRF_ERROR_NULL;
    }

    if (m_event_handler != NULL)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    m_event_handler = event_handler;

    if (m_uart_port_name[0] == '\0')
    {
        app_uart_port_name_set(DEFAULT_PORT_NAME);
    }

    if (mp_uart_object != NULL)
    {
        delete mp_uart_object;
    }

    UartCommunicationParameters uart_comm_params;

    fill_communications_struct(&uart_comm_params);

    mp_uart_object = new UartBoost(data_received, data_sent);
    mp_uart_object->open(uart_comm_params);

    if (!mp_uart_object->isOpen())
    {
        app_log_handler(APP_LOG_ERROR, "Unable to open serial port %s.", m_uart_port_name);

        return NRF_ERROR_INVALID_STATE;
    }

    return NRF_SUCCESS;
}

uint32_t app_uart_get(uint8_t * p_byte)
{
    UNUSED_PARAMETER(p_byte);

    return NRF_ERROR_NOT_SUPPORTED;
}

uint32_t app_uart_put(uint8_t byte)
{
    if (mp_uart_object == NULL)
    {
        app_log_handler(APP_LOG_WARNING, "Could not write to serial port.");
        return NRF_ERROR_INVALID_STATE;
    }

    return mp_uart_object->write(byte);
}

uint32_t app_uart_get_connection_state(app_uart_connection_state_t * p_connection_state)
{
    UNUSED_PARAMETER(p_connection_state);

    if (mp_uart_object == NULL)
    {
        *p_connection_state = APP_UART_DISCONNECTED;
    }
    else if(mp_uart_object->isOpen())
    {
        *p_connection_state = APP_UART_CONNECTED;
    }
    else
    {
        *p_connection_state = APP_UART_DISCONNECTED;
    }

    return NRF_SUCCESS;
}

uint32_t app_uart_flush(void)
{
    if (mp_uart_object != NULL)
    {
        return mp_uart_object->flush();
    }

    app_log_handler(APP_LOG_INFO, "Could not flush UART.");
    return NRF_ERROR_INVALID_STATE;
}

uint32_t app_uart_close(uint16_t app_uart_id)
{
    UNUSED_PARAMETER(app_uart_id);

    if (mp_uart_object == NULL)
    {
        app_log_handler(APP_LOG_INFO, "Could not close serial port.");
        return NRF_ERROR_INVALID_STATE;
    }

    uint32_t returnValue = mp_uart_object->close();
    delete mp_uart_object;
    mp_uart_object = NULL;
    m_event_handler = NULL;
    return returnValue;
}
