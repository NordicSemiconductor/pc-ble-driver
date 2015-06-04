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

#ifndef APP_TIMER_EXTENSION_H__
#define APP_TIMER_EXTENSION_H__

/**@brief Closes the timer and releases the resources.
 */
void app_timer_close();

/**@brief Set the timeout of the timer in milliseconds.
 *
 * @param[in]  timeout_ms   timeout value in milliseconds
 */
void app_timer_timeout_ms_set(uint32_t timeout_ms);

#endif // APP_TIMER_EXTENSION_H__
