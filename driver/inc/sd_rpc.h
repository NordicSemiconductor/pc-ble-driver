/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic Semiconductor. The use,
 * copying, transfer or disclosure of such information is prohibited except by express written
 * agreement with Nordic Semiconductor.
 *
 */
/** @file
 *
 * @brief Type definitions and API calls for SoftDevice RPC module.
 *
 */

#ifndef SD_RPC_H__
#define SD_RPC_H__

#include "nrf_svc.h"
#include "ble.h"

/**@brief Levels of severity that a log message can be associated with. */
typedef enum
{
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
} sd_rpc_log_severity_t;

/**@brief Flow control modes */
typedef enum
{
    FLOW_CONTROL_NONE,
    FLOW_CONTROL_HARDWARE
} sd_rpc_flow_control_t;

/**@brief Parity modes */
typedef enum
{
    PARITY_NONE,
    PARITY_EVEN
} sd_rpc_parity_t;

/**@brief Function pointer type for log callbacks.
 */
typedef void (*sd_rpc_log_handler_t)(sd_rpc_log_severity_t severity, const char * log_message);

/**@brief Function pointer type for event callbacks.
 */
typedef void (*sd_rpc_evt_handler_t)(ble_evt_t * p_ble_evt);

/**@brief Initialize the SoftDevice RPC module.
 *
 * @note This function must be called prior to the sd_ble_* API commands.
 *       The serial port will be attempted opened with the configured serial port settings.
 *
 * @retval NRF_SUCCESS  The module was opened successfully.
 * @retval NRF_ERROR    There was an error opening the module.
 */
SD_RPC_API uint32_t sd_rpc_open();

/**@brief Close the SoftDevice RPC module.
 *
 * @note This function will close the serial port and release allocated resources.
 *
 * @retval NRF_SUCCESS  The module was closed successfully.
 * @retval NRF_ERROR    There was an error closing the module.
 */
SD_RPC_API uint32_t sd_rpc_close();


/**@brief Register a handler function for log callbacks.
 *        Default is no log handler.
 *
 * @param[in]  log_handler  Function that will be called when a log message is sent out.
 *
 * @retval NRF_SUCCESS      This function will only return success.
 */
SD_RPC_API uint32_t sd_rpc_log_handler_set(sd_rpc_log_handler_t log_handler);

/**@breif Register file path for log file.
 *        Default is no log file.
 *
 * @param[in]  file_path  Path to the log file.
 *
 * @retval NRF_SUCCESS              file_path is valid.
 * @retval NRF_ERROR_INVALID_PARAM  file_path does not point at an existing directory with a valid
 *                                  file name.
 */
SD_RPC_API uint32_t sd_rpc_log_file_path_set(const char * file_path);

/**@brief Set the lowest log level for messages to be logged to handler.
 *        Default log handler severity filter is LOG_INFO.
 *
 * @param[in]  severity_filter  The lowest severity level messages should be logged.
 *
 * @retval NRF_SUCCESS              severity_filter is valid.
 * @retval NRF_ERROR_INVALID_PARAM  severity_filter is not one of the valid enum values
 *                                  in app_log_severity_t
 */
SD_RPC_API uint32_t sd_rpc_log_handler_severity_filter_set(sd_rpc_log_severity_t severity_filter);

/**@brief Set the lowest log level for messages to be logged to file.
 *        Default log file severity filter is LOG_TRACE.
 *
 * @param[in]  severity_filter  The lowest severity level messages should be logged.
 *
 * @retval NRF_SUCCESS              severity_filter is valid.
 * @retval NRF_ERROR_INVALID_PARAM  severity_filter is not one of the valid enum values
 *                                  in app_log_severity_t
 */
SD_RPC_API uint32_t sd_rpc_log_file_severity_filter_set(sd_rpc_log_severity_t severity_filter);

/**@brief Register a handler function for events coming from the SoftDevice.
 *        Default is no event handler.
 *
 * @param[in]  event_handler  Function that will be called when an event is sent out.
 *
 * @retval NRF_SUCCESS  This function will only return success.
 */
SD_RPC_API uint32_t sd_rpc_evt_handler_set(sd_rpc_evt_handler_t event_handler);

/**@brief Set the port name to be used for UART communication with nRF51 device.
 *        Default serial port name is COM1.
 *
 * @param[in]  port_name  Port name to be used for UART communication.
 *
 * @retval NRF_SUCCESS  This function will only return success,
 *                      opening the port may still fail.
 */
SD_RPC_API uint32_t sd_rpc_serial_port_name_set(const char * port_name);

/**@brief Set the baud rate to be used for UART communication with nRF51 device.
 *        Default baud rate is 1000000.
 *
 * @note Valid baud rates are: 1200, 2400, 4800, 9600, 14400, 19200, 28800,
 *       38400, 57600, 76800, 115200, 230400, 250000, 460800, 921600, 1000000
 *
 * @param[in]  baud_rate  Baud rate to be used for UART communication.
 *
 * @retval NRF_SUCCESS              baud_rate is valid.
 * @retval NRF_ERROR_INVALID_PARAM  baud_rate is not one of the valid rates.
 */
SD_RPC_API uint32_t sd_rpc_serial_baud_rate_set(uint32_t baud_rate);

/**@brief Set the flow control to be used for UART communication with nRF51 device.
 *        Default flow control is FLOW_CONTROL_HARDWARE.
 *
 * @param[in]  flow_control  If flow control should be enabled or not for UART communication.
 *
 * @retval NRF_SUCCESS              flow_control is valid.
 * @retval NRF_ERROR_INVALID_PARAM  flow_control is not one of the valid enum values
 *                                  in sd_rpc_flow_control_t
 */
SD_RPC_API uint32_t sd_rpc_serial_flow_control_set(sd_rpc_flow_control_t flow_control);

/**@brief Set the parity to be used for UART communication with nRF51 device.
 *        By default parity is used.
 *
 * @param[in]  parity  If parity should be enabled or not for UART communication.
 *
 * @retval NRF_SUCCESS              parity is valid.
 * @retval NRF_ERROR_INVALID_PARAM  parity is not one of the valid enum values in sd_rpc_parity_t
 */
SD_RPC_API uint32_t sd_rpc_serial_parity_set(sd_rpc_parity_t parity);

#endif
