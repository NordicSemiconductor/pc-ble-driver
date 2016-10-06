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
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (*field_encoder_handler_t)(void const * const p_field,
                                            uint8_t * const    p_buf,
                                            uint32_t           buf_len,
                                            uint32_t * const   p_index);

typedef uint32_t (*field_decoder_handler_t)(uint8_t const * const p_buf,
                                            uint32_t              buf_len,
                                            uint32_t * const      p_index,
                                            void *                p_field);

/**@brief Function for safe encoding conditional field.
 *
 * Function sets 'presence flag' and checks if conditional field is provided and if it is not NULL
 * it calls provided parser function which attempts to encode field content to the buffer stream.
 *
 * @param[in]      p_field          Pointer to input struct.
 * @param[in]      p_buf            Pointer to the beginning of the output buffer.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the encoded data.
 * @param[in]      fp_field_encoder Pointer to the function which implements fields encoding.
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t cond_field_enc(void const * const      p_field,
                        uint8_t * const         p_buf,
                        uint32_t                buf_len,
                        uint32_t * const        p_index,
                        field_encoder_handler_t field_parser);

/**@brief Function for safe decoding conditional field.
 *
 * Function checks if conditional field is present in the input buffer and if it is set it calls
 * provided parser function which attempts to parse buffer content to the known field.
 *
 * @param[in]      p_buf            Pointer to the beginning of the input buffer.
 * @param[in]      buf_len          Size of buffer.
 * @param[in,out]  p_index          \c in: Index to start of uint8 value in buffer.
 *                                  \c out: Index in buffer to first byte after the decoded data.
 * @param[in]      pp_field         Pointer to pointer to output location.
 * @param[in]      fp_field_decoder Pointer to the function which implements field decoding.
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t cond_field_dec(uint8_t const * const   p_buf,
                        uint32_t                buf_len,
                        uint32_t * const        p_index,
                        void * * const          pp_field,
                        field_decoder_handler_t field_parser);

#ifdef __cplusplus
}
#endif
