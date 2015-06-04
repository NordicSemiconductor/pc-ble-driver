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

#ifndef COMMAND_LOGGING_H__
#define COMMAND_LOGGING_H__

#include <stdint.h>


/**@brief Logs the name of a command used in a command call.
 *
 * @param[in]  uint16_t command_number  command number of the command add to log
 */
void app_log_utility_command_log(uint16_t command_number);

/**@brief Log the name of a command received in a command response.
 *
 * @param[in]  uint16_t command_number  command number of the command add to log
 */
void app_log_utility_command_response_log(uint16_t command_number);

/**@brief Logs the name of an event.
 *
 * @param[in]  uint16_t event_number    event nuber of the event to log
 */
void app_log_utility_event_log(uint16_t event_number);

#endif // COMMAND_LOGGING_H__
