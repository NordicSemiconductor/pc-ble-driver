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

#ifndef SER_SOFTDEVICE_HANDLER_EXTENSION_H__
#define SER_SOFTDEVICE_HANDLER_EXTENSION_H__

/**@brief Returns the length of the queue.
 *
 * @param[out]  uint32_t mailbox_length     length of the mailbox queue
 */
uint32_t sd_ble_evt_mailbox_length_get(uint32_t * mailbox_length);

#endif // SER_SOFTDEVICE_HANDLER_EXTENSION_H__
