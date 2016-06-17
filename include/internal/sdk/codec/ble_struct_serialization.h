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

#include "ble_types.h"


#ifdef __cplusplus
extern "C" {
#endif

uint32_t ble_uuid_t_enc(void const * const p_void_uuid,
                        uint8_t * const    p_buf,
                        uint32_t           buf_len,
                        uint32_t * const   p_index);

uint32_t ble_uuid_t_dec(uint8_t const * const p_buf,
                        uint32_t              buf_len,
                        uint32_t * const      p_index,
                        void * const          p_void_uuid);

uint32_t ble_uuid128_t_enc(const void * const p_void_uuid,
                           uint8_t * const    p_buf,
                           uint32_t           buf_len,
                           uint32_t * const   p_index);

uint32_t ble_uuid128_t_dec(uint8_t const * const p_buf,
                           uint32_t              buf_len,
                           uint32_t * const      p_index,
                           void * const          p_void_uuid);

uint32_t ble_l2cap_header_t_enc(void const * const p_void_header,
                                uint8_t * const    p_buf,
                                uint32_t           buf_len,
                                uint32_t * const   p_index);

uint32_t ble_l2cap_header_t_dec(uint8_t const * const p_buf,
                                uint32_t              buf_len,
                                uint32_t * const      p_index,
                                void * const          p_void_header);

uint32_t ble_l2cap_evt_rx_t_enc(void const * const p_void_evt_rx,
                                uint8_t * const    p_buf,
                                uint32_t           buf_len,
                                uint32_t * const   p_index);

uint32_t ble_l2cap_evt_rx_t_dec(uint8_t const * const p_buf,
                                uint32_t              buf_len,
                                uint32_t * const      p_index,
                                uint32_t * const      p_event_len,
                                void * const          p_void_evt_rx);
                                
uint32_t ble_enable_params_t_enc(void const * const p_void_enable_params,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index);
                                 
uint32_t ble_enable_params_t_dec(uint8_t const * const p_buf,
                                 uint32_t              buf_len,
                                 uint32_t * const      p_index,
                                 void * const          p_void_enable_params);

uint32_t ble_conn_bw_t_enc(void const * const p_void_conn_bw,
                           uint8_t * const    p_buf,
                           uint32_t           buf_len,
                           uint32_t * const   p_index);

uint32_t ble_conn_bw_t_dec(uint8_t const * const p_buf,
                           uint32_t              buf_len,
                           uint32_t * const      p_index,
                           void * const          p_void_conn_bw);

uint32_t ble_common_opt_conn_bw_t_enc(void const * const p_void_opt_conn_bw,
                                      uint8_t * const    p_buf,
                                      uint32_t           buf_len,
                                      uint32_t * const   p_index);

uint32_t ble_common_opt_conn_bw_t_dec(uint8_t const * const p_buf,
                                      uint32_t              buf_len,
                                      uint32_t * const      p_index,
                                      void * const          p_void_opt_conn_bw);

uint32_t ble_conn_bw_count_t_enc(void const * const p_void_conn_bw_count,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index);

uint32_t ble_conn_bw_count_t_dec(uint8_t const * const p_buf,
                                 uint32_t              buf_len,
                                 uint32_t * const      p_index,
                                 void * const          p_void_conn_bw_count);

uint32_t ble_conn_bw_counts_t_enc(void const * const p_void_conn_bw_count,
                                  uint8_t * const    p_buf,
                                  uint32_t           buf_len,
                                  uint32_t * const   p_index);

uint32_t ble_conn_bw_counts_t_dec(uint8_t const * const p_buf,
                                  uint32_t              buf_len,
                                  uint32_t * const      p_index,
                                  void * const          p_void_conn_bw_count);

uint32_t ble_common_enable_params_t_enc(void const * const p_void_common_enable_params,
                                        uint8_t * const    p_buf,
                                        uint32_t           buf_len,
                                        uint32_t * const   p_index);

uint32_t ble_common_enable_params_t_dec(uint8_t const * const p_buf,
                                        uint32_t              buf_len,
                                        uint32_t * const      p_index,
                                        void * const          p_void_common_enable_params);

uint32_t ble_common_opt_pa_lna_t_enc(void const * const p_void_opt,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index);

uint32_t ble_common_opt_pa_lna_t_dec(uint8_t const * const p_buf,
                                      uint32_t              buf_len,
                                      uint32_t * const      p_index,
                                      void * const          p_void_opt);

uint32_t ble_pa_lna_cfg_t_enc(void const * const p_void_cfg,
                                 uint8_t * const    p_buf,
                                 uint32_t           buf_len,
                                 uint32_t * const   p_index);

uint32_t ble_pa_lna_cfg_t_dec(uint8_t const * const p_buf,
                                      uint32_t              buf_len,
                                      uint32_t * const      p_index,
                                      void * const          p_void_cfg);

#ifdef __cplusplus
}
#endif
