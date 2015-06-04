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

#include "sd_rpc.h"
#include "app_log.h"

#include "ble.h"

#include "nrf_error.h"
#include "softdevice_handler.h"

#include "app_timer_extension.h"
#include "app_uart_extension.h"
#include "ser_app_hal_pc_extension.h"

 /**@brief Macro for calling error handler function if supplied error code any other than NRF_SUCCESS.
 *
 * @param[in] ERR_CODE Error code supplied to the error handler.
 */
#define ERROR_CHECK(ERR_CODE)                                                   \
    do                                                                          \
    {                                                                           \
        const uint32_t LOCAL_ERR_CODE = (ERR_CODE);                             \
        if (LOCAL_ERR_CODE != NRF_SUCCESS)                                      \
        {                                                                       \
            app_error_handler((LOCAL_ERR_CODE), __LINE__, (uint8_t*) __FILE__); \
            return LOCAL_ERR_CODE;                                              \
        }                                                                       \
    } while (0)

void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    app_log_handler(APP_LOG_ERROR, "File: %s, Line: %u, Error code: %u",
                    p_file_name, line_num, error_code);
}

void assert_nrf_callback(uint16_t line_num, const uint8_t * file_name)
{
    app_error_handler(NRF_ERROR_INTERNAL, line_num, file_name);
}

uint32_t sd_rpc_open()
{
    ser_app_hal_pc_init();

    uint32_t error_code;
    nrf_clock_lfclksrc_t clock_source = 0;

    static uint32_t ble_evt_buffer[CEIL_DIV(BLE_STACK_EVT_MSG_BUF_SIZE, sizeof(uint32_t))];
    error_code = softdevice_handler_init((clock_source),
                                       ble_evt_buffer,
                                       sizeof(ble_evt_buffer),
                                       NULL);

    if (error_code != NRF_SUCCESS)
    {
        return error_code;
    }

    // Check if the connectivity firmware gives a valid response.
    ble_version_t version;
    error_code = sd_ble_version_get(&version);

    if (error_code != NRF_SUCCESS &&
        error_code != NRF_ERROR_INVALID_STATE &&
        error_code != BLE_ERROR_NOT_ENABLED)
    {
        return error_code;
    }

    return NRF_SUCCESS;
}

uint32_t sd_rpc_close()
{
    ser_app_hal_pc_event_handling_stop();

    sd_softdevice_disable();

    return NRF_SUCCESS;
}

uint32_t sd_rpc_log_handler_set(sd_rpc_log_handler_t log_handler)
{
    return app_log_handler_set((app_log_handler_t)log_handler);
}

uint32_t sd_rpc_log_file_path_set(const char * file_path)
{
    return app_log_file_path_set(file_path);
}

uint32_t sd_rpc_log_handler_severity_filter_set(sd_rpc_log_severity_t severity_filter)
{
    return app_log_handler_severity_filter_set((app_log_severity_t)severity_filter);
}

uint32_t sd_rpc_log_file_severity_filter_set(sd_rpc_log_severity_t severity_filter)
{
    return app_log_file_severity_filter_set((app_log_severity_t)severity_filter);
}

uint32_t sd_rpc_evt_handler_set(sd_rpc_evt_handler_t event_handler)
{
    return softdevice_ble_evt_handler_set(event_handler);
}

uint32_t sd_rpc_serial_port_name_set(const char * port_name)
{
    return  app_uart_port_name_set(port_name);
}

uint32_t sd_rpc_serial_baud_rate_set(uint32_t baud_rate)
{
    // Baud rate 1000000 gives by sdk calculations a timeout of ~3.9ms.
    uint32_t bits_before_timeout = 10000000;
    uint32_t timeout_in_ms = bits_before_timeout / baud_rate;
    app_timer_timeout_ms_set(timeout_in_ms);
    return app_uart_baud_rate_set(baud_rate);
}

uint32_t sd_rpc_serial_flow_control_set(sd_rpc_flow_control_t flow_control)
{
    if (flow_control == FLOW_CONTROL_NONE)
    {
        return  app_uart_flow_control_set(APP_UART_FLOW_CONTROL_NONE);
    }
    else if (flow_control == FLOW_CONTROL_HARDWARE)
    {
        return  app_uart_flow_control_set(APP_UART_FLOW_CONTROL_HARDWARE);
    }

    return NRF_ERROR_INVALID_PARAM;
}

uint32_t sd_rpc_serial_parity_set(sd_rpc_parity_t parity)
{
    if (parity == PARITY_NONE)
    {
        return  app_uart_parity_set(APP_UART_PARITY_NONE);
    }
    else if (parity == PARITY_EVEN)
    {
        return  app_uart_parity_set(APP_UART_PARITY_EVEN);
    }

    return NRF_ERROR_INVALID_PARAM;
}
