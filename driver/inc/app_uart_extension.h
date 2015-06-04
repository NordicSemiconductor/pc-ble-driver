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

#ifndef APP_UART_EXTENSION_H__
#define APP_UART_EXTENSION_H__

#include <stdint.h>


/**@brief UART flow control modes. */
typedef enum
{
    APP_UART_FLOW_CONTROL_NONE,
    APP_UART_FLOW_CONTROL_HARDWARE
} app_uart_extension_flow_control_t;

/**@brief UART parity modes. */
typedef enum
{
    APP_UART_PARITY_NONE,
    APP_UART_PARITY_ODD,
    APP_UART_PARITY_EVEN
} app_uart_extension_parity_t;

/**@brief Function prototype for received packet callback. */
typedef void (*app_uart_packet_received_handler_t)();

/**@brief Set the name of the serial port
 *
 * @param[in] port_name         The name of the serial port that will be use.
 *
 * @return @ref NRF_SUCCESS     Port name is set, no check if it is a valid name.
 */
uint32_t app_uart_port_name_set(const char * port_name);

/**@brief Set the baud rate of the serial port
 *
 * @param[in] baud_rate                     The baud rate the serial port will use.
 *
 * @return @ref NRF_ERROR_INVALID_PARAM     Baud rate is not set, not one of the valid baud rates.
 * @return @ref NRF_SUCCESS                 Baud rate is set.
 */
uint32_t app_uart_baud_rate_set(uint32_t baud_rate);

/**@brief Set the flow control of the serial port
 *
 * @param[in] flow_control                  The flow control the serial port will use.
 *
 * @return @ref NRF_ERROR_INVALID_PARAM     Flow control is not set, not a valid flow control.
 * @return @ref NRF_SUCCESS                 Flow control is set.
 */
uint32_t app_uart_flow_control_set(app_uart_extension_flow_control_t flow_control);

/**@brief Set the parity of the serial port
 *
 * @param[in] parity                        The parity the serial port will use.
 *
 * @return @ref NRF_ERROR_INVALID_PARAM     Parity is not set, not a valid parity.
 * @return @ref NRF_SUCCESS                 Parity is set.
 */
uint32_t app_uart_parity_set(app_uart_extension_parity_t parity);


#endif // APP_UART_EXTENSION_H__
