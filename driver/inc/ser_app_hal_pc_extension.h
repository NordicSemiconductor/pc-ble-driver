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

#ifndef SER_APP_HAL_PC_EXTENSION_H__
#define SER_APP_HAL_PC_EXTENSION_H__

/**@brief Logs the name of a command used in a command call.
 *
 * @param[in]  uint16_t command_number  command number of the command add to log
 */
void packet_received_handler();

/**@brief Stops the event handling thread.
 */
void ser_app_hal_pc_event_handling_stop();

/**@brief Starts the event handling thread.
 */
void ser_app_hal_pc_init();

#endif // SER_APP_HAL_PC_EXTENSION_H__
