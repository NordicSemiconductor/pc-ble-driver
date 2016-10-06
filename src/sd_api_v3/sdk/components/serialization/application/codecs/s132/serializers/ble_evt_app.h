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
#ifndef BLE_EVT_APP_H__
#define BLE_EVT_APP_H__

/**
 * @addtogroup ser_codecs Serialization codecs
 * @ingroup ble_sdk_lib_serialization
 */

/**
 * @addtogroup ser_app_s130_codecs Application S132 codecs
 * @ingroup ser_codecs
 */

/**@file
 *
 * @defgroup ble_evt_app Application event decoders
 * @{
 * @ingroup  ser_app_s130_codecs
 *
 * @brief    Application event decoders.
 */
#include "ble.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Decodes the ble_evt_tx_complete event.
 *
 * If \p p_event is null, the required length of \p p_event is returned in \p p_event_len.
 *
 * @param[in] p_buf            Pointer to the beginning of the event packet.
 * @param[in] packet_len       Length (in bytes) of the event packet.
 * @param[in,out] p_event      Pointer to the \ref ble_evt_t buffer where the decoded event will be
 *                             stored. If NULL, the required length will be returned in \p p_event_len.
 * @param[in,out] p_event_len  \c in: Size (in bytes) of the \p p_event buffer.
 *                             \c out: Length of the decoded contents of \p p_event.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_DATA_SIZE       Decoding failure. Length of \p p_event is too small to
 *                                   hold the decoded event.
 */
uint32_t ble_evt_tx_complete_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 ble_evt_t * const     p_event,
                                 uint32_t * const      p_event_len);

/**
 * @brief Decodes the ble_evt_user_mem_request event.
 *
 * If \p p_event is null, the required length of \p p_event is returned in \p p_event_len.
 *
 * @param[in] p_buf            Pointer to the beginning of the event packet.
 * @param[in] packet_len       Length (in bytes) of the event packet.
 * @param[in,out] p_event      Pointer to the \ref ble_evt_t buffer where the decoded event will be
 *                             stored. If NULL, the required length will be returned in \p p_event_len.
 * @param[in,out] p_event_len  \c in: Size (in bytes) of the \p p_event buffer.
 *                             \c out: Length of the decoded contents of \p p_event.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_DATA_SIZE       Decoding failure. Length of \p p_event is too small to
 *                                   hold the decoded event.
 */
uint32_t ble_evt_user_mem_request_dec(uint8_t const * const p_buf,
                                      uint32_t              packet_len,
                                      ble_evt_t * const     p_event,
                                      uint32_t * const      p_event_len);

/**
 * @brief Decodes the ble_evt_user_mem_release event.
 *
 * If \p p_event is null, the required length of \p p_event is returned in \p p_event_len.
 *
 * @param[in] p_buf            Pointer to the beginning of the event packet.
 * @param[in] packet_len       Length (in bytes) of the event packet.
 * @param[in,out] p_event      Pointer to the \ref ble_evt_t buffer where the decoded event will be
 *                             stored. If NULL, the required length will be returned in \p p_event_len.
 * @param[in,out] p_event_len  \c in: Size (in bytes) of the \p p_event buffer.
 *                             \c out: Length of the decoded contents of \p p_event.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_DATA_SIZE       Decoding failure. Length of \p p_event is too small to
 *                                   hold the decoded event.
 */
uint32_t ble_evt_user_mem_release_dec(uint8_t const * const p_buf,
                                      uint32_t              packet_len,
                                      ble_evt_t * const     p_event,
                                      uint32_t * const      p_event_len);

/**
 * @brief Decodes the ble_evt_data_length_changed event.
 *
 * If \p p_event is null, the required length of \p p_event is returned in \p p_event_len.
 *
 * @param[in] p_buf            Pointer to the beginning of the event packet.
 * @param[in] packet_len       Length (in bytes) of the event packet.
 * @param[in,out] p_event      Pointer to the \ref ble_evt_t buffer where the decoded event will be
 *                             stored. If NULL, the required length will be returned in \p p_event_len.
 * @param[in,out] p_event_len  \c in: Size (in bytes) of the \p p_event buffer.
 *                             \c out: Length of the decoded contents of \p p_event.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_DATA_SIZE       Decoding failure. Length of \p p_event is too small to
 *                                   hold the decoded event.
 */
uint32_t ble_evt_data_length_changed_dec(uint8_t const * const p_buf,
                                         uint32_t              packet_len,
                                         ble_evt_t * const     p_event,
                                         uint32_t * const      p_event_len);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
