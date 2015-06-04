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

#ifndef APP_LOG_H__
#define APP_LOG_H__

#include <stdint.h>
#include "nrf_svc.h"

/**@brief The levels of severity that a log message can be associated with. */
typedef enum
{
    APP_LOG_TRACE,
    APP_LOG_DEBUG,
    APP_LOG_INFO,
    APP_LOG_WARNING,
    APP_LOG_ERROR,
    APP_LOG_FATAL
} app_log_severity_t;


/**@brief Function pointer definition for use with log message callback.
*
* @param[in] severity       Log message severity.
* @param[in} log_message    Log message string.
*/
typedef void (*app_log_handler_t)(app_log_severity_t severity, const char * log_message);


/**@brief Function for logging text message.
 *
 * @param[in] severity    Log message severity.
 * @param[in] log_message Log message string.
 * @param[in] byte_array  Byte array that will be logged as hex character text.
 */
void app_log_byte_array_handler(const app_log_severity_t severity, const char * log_message,
                                const uint8_t * byte_array, const uint16_t array_length);

/**@brief Function for logging text message and byte array.
 *
 * @param[in] severity    Log message severity.
 * @param[in] log_message Log message string.
 */
void app_log_handler(app_log_severity_t severity, const char * log_message, ...);

/**@brief Register a handler function for log callbacks.
 *
 * @param[in]  log_handler    Function that will be called when a log message is sent out.
 *
 * @retval NRF_SUCCESS        This function will only return success.
 */
uint32_t app_log_handler_set(app_log_handler_t log_handler);

/**@brief Register file path for log file.
 *
 * @param[in]  file_path    Path to the log file.
 *
 * @retval NRF_SUCCESS              file_path is valid.
 * @retval NRF_ERROR_INVALID_PARAM  file_path does not point at an existing directory with a valid
 *                                  file name.
 */
uint32_t app_log_file_path_set(const char * file_path);

/**@brief Set the lowest log level for messages to be logged to handler.
 *
 * @param[in]  severity_filter  The lowest severity level messages should be logged.
 *
 * @retval NRF_SUCCESS              severity_filter is valid.
 * @retval NRF_ERROR_INVALID_PARAM  severity_filter is not one of the valid enum values
 *                                  in app_log_severity_t
 */
uint32_t app_log_handler_severity_filter_set(app_log_severity_t severity_filter);

/**@brief Set the lowest log level for messages to be logged to file.
 *
 * @param[in]  severity_filter  The lowest severity level messages should be logged.
 *
 * @retval NRF_SUCCESS              severity_filter is valid.
 * @retval NRF_ERROR_INVALID_PARAM  severity_filter is not one of the valid enum values
 *                                  in app_log_severity_t
 */
uint32_t app_log_file_severity_filter_set(app_log_severity_t severity_filter);

#endif // APP_LOG_H__
