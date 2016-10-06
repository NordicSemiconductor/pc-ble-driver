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

#ifndef COND_FIELD_SERIALIZATION_H__
#define COND_FIELD_SERIALIZATION_H__

#include <stdint.h>

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

/**@brief Function for safe encoding of a conditional field.
 *
 * Function sets a presence flag and checks if conditional field is provided. If the field is not NULL,
 * it calls the provided parser function which attempts to encode the field content to the buffer stream.
 *
 * @param[in]      p_field          Pointer to the input struct.
 * @param[in]      p_buf            Pointer to the beginning of the output buffer.
 * @param[in]      buf_len          Size of the buffer.
 * @param[in,out]  p_index          \c in: Index to the start of the uint8 value in the buffer.
 *                                  \c out: Index in the buffer to the first byte after the encoded data.
 * @param[in]      fp_field_encoder Pointer to the function which implements fields encoding.
 *
 * @return NRF_SUCCESS              Fields encoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t cond_field_enc(void const * const      p_field,
                        uint8_t * const         p_buf,
                        uint32_t                buf_len,
                        uint32_t * const        p_index,
                        field_encoder_handler_t fp_field_encoder);

/**@brief Function for safe decoding of a conditional field.
 *
 * Function checks if conditional field is present in the input buffer. If it is set, it calls
 * the provided parser function which attempts to parse the buffer content to the known field.
 *
 * @param[in]      p_buf            Pointer to the beginning of the input buffer.
 * @param[in]      buf_len          Size of the buffer.
 * @param[in,out]  p_index          \c in: Index to the start of the uint8 value in the buffer.
 *                                  \c out: Index in the buffer to the first byte after the decoded data.
 * @param[in]      pp_field         Pointer to output location.
 * @param[in]      fp_field_decoder Pointer to the function which implements field decoding.
 *
 * @return NRF_SUCCESS              Fields decoded successfully.
 * @retval NRF_ERROR_INVALID_LENGTH Decoding failure. Incorrect buffer length.
 */
uint32_t cond_field_dec(uint8_t const * const   p_buf,
                        uint32_t                buf_len,
                        uint32_t * const        p_index,
                        void * * const          pp_field,
                        field_decoder_handler_t fp_field_decoder);

#ifdef __cplusplus
}
#endif

#endif // COND_FIELD_SERIALIZATION_H__
