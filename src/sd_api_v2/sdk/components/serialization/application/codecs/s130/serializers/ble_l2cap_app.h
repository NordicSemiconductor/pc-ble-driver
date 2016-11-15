/*
 * Copyright (c) 2016 Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 *   3. Neither the name of Nordic Semiconductor ASA nor the names of other
 *   contributors to this software may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *   4. This software must only be used in or with a processor manufactured by Nordic
 *   Semiconductor ASA, or in or with a processor manufactured by a third party that
 *   is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 *   5. Any software provided in binary or object form under this license must not be
 *   reverse engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @addtogroup ser_codecs Serialization codecs
 * @ingroup ble_sdk_lib_serialization
 */

/**
 * @addtogroup ser_app_s130_codecs Application s130 codecs
 * @ingroup ser_codecs
 */

/**@file
 *
 * @defgroup ble_l2cap_app L2CAP Application command request encoders and command response decoders
 * @{
 * @ingroup  ser_app_s130_codecs
 *
 * @brief    L2CAP Application command request encoders and command response decoders.
 */

#ifndef BLE_L2CAP_APP_H__
#define BLE_L2CAP_APP_H__

#include "ble.h"
#include "ble_types.h"
#include "ble_ranges.h"
#include "ble_err.h"
#include "ble_l2cap.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@brief Register a CID with L2CAP.
 *
 * @details This registers a higher protocol layer with the L2CAP multiplexer, and is requried prior to all operations on the CID.
 *
 * @param[in]     cid L2CAP CID.
 * @param[in]     p_buf          Pointer to beginning of command response packet.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_l2cap_cid_register_req_enc(uint16_t         cid,
                                        uint8_t * const  p_buf,
                                        uint32_t * const p_buf_len);

/**
 * @brief Decodes response to @ref sd_ble_l2cap_cid_register command.
 *
 * @sa @ref nrf51_adv_start_encoding for packet format,
 *     @ref ble_l2cap_cid_register_req_enc for command request encoder.
 *
 * @param[in]  p_buf          Pointer to beginning of command response packet.
 * @param[in]  packet_len     Length (in bytes) of response packet.
 * @param[out] p_result_code  Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match
 *                                   expected operation code.
 */
uint32_t ble_l2cap_cid_register_rsp_dec(uint8_t const * const p_buf,
                                        uint32_t              packet_len,
                                        uint32_t * const      p_result_code);

/**@brief Unregister a CID with L2CAP.
 *
 * @details This unregisters a previously registered higher protocol layer with the L2CAP multiplexer.
 *
 * @param[in]     cid L2CAP CID.
 * @param[in]     p_buf          Pointer to beginning of command response packet.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @retval NRF_SUCCESS                Encoding success.
 * @retval NRF_ERROR_NULL             Encoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH   Encoding failure. Incorrect buffer length.
 */
uint32_t ble_l2cap_cid_unregister_req_enc(uint16_t         cid,
                                          uint8_t * const  p_buf,
                                          uint32_t * const p_buf_len);

/**
 * @brief Decodes response to @ref sd_ble_l2cap_cid_unregister command.
 *
 * @sa @ref nrf51_adv_start_encoding for packet format,
 *     @ref ble_l2cap_cid_unregister_req_enc for command request encoder.
 *
 * @param[in]  p_buf          Pointer to beginning of command response packet.
 * @param[in]  packet_len     Length (in bytes) of response packet.
 * @param[out] p_result_code  Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match
 *                                   expected operation code.
 */
uint32_t ble_l2cap_cid_unregister_rsp_dec(uint8_t const * const p_buf,
                                          uint32_t              packet_len,
                                          uint32_t * const      p_result_code);

/**@brief Transmit an L2CAP packet.
 *
 * @note    It is important to note that a call to this function will <b>consume an application buffer</b>, and will therefore
 *          generate a @ref BLE_EVT_TX_COMPLETE event when the packet has been transmitted.
 *          Please see the documentation of @ref sd_ble_tx_buffer_count_get for more details.
 *
 * @param[in]     conn_handle Connection Handle.
 * @param[in]     p_l2cap_header    Pointer to a packet header containing length and CID.
 * @param[in]     p_data      Pointer to the data to be transmitted.
 * @param[in]     p_buf          Pointer to beginning of command response packet.
 * @param[in,out] p_buf_len  \c in: Size of \p p_buf buffer.
 *                           \c out: Length of encoded command packet.
 *
 * @return @ref NRF_SUCCESS Successfully queued an L2CAP packet for transmission.
 * @return @ref NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 * @return @ref NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied, CIDs must be registered beforehand with @ref sd_ble_l2cap_cid_register.
 * @return @ref NRF_ERROR_NOT_FOUND CID not found.
 * @return @ref NRF_ERROR_NO_MEM Not enough memory to complete operation.
 * @return @ref BLE_ERROR_NO_TX_BUFFERS Not enough application buffers available.
 * @return @ref NRF_ERROR_DATA_SIZE Invalid data size(s) supplied, see @ref BLE_L2CAP_MTU_DEF.
 */
uint32_t ble_l2cap_tx_req_enc(uint16_t                         conn_handle,
                              ble_l2cap_header_t const * const p_l2cap_header,
                              uint8_t const * const            p_data,
                              uint8_t * const                  p_buf,
                              uint32_t * const                 p_buf_len);

/**
 * @brief Decodes response to @ref sd_ble_l2cap_tx command.
 *
 * @sa @ref nrf51_adv_start_encoding for packet format,
 *     @ref ble_l2cap_tx_req_enc for command request encoder.
 *
 * @param[in]  p_buf          Pointer to beginning of command response packet.
 * @param[in]  packet_len     Length (in bytes) of response packet.
 * @param[out] p_result_code  Command result code.
 *
 * @retval NRF_SUCCESS               Decoding success.
 * @retval NRF_ERROR_NULL            Decoding failure. NULL pointer supplied.
 * @retval NRF_ERROR_INVALID_LENGTH  Decoding failure. Incorrect buffer length.
 * @retval NRF_ERROR_INVALID_DATA    Decoding failure. Decoded operation code does not match
 *                                   expected operation code.
 */
uint32_t ble_l2cap_tx_rsp_dec(uint8_t const * const p_buf,
                              uint32_t              packet_len,
                              uint32_t * const      p_result_code);


#ifdef __cplusplus
}
#endif
#endif //BLE_L2CAP_APP_H__

/**
   @}
 */
