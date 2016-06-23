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
#include "nrf_error.h"
#include "cond_field_serialization.h"
#include "ble_serialization.h"
#include <stddef.h>

uint32_t cond_field_enc(void const * const      p_field,
                        uint8_t * const         p_buf,
                        uint32_t                buf_len,
                        uint32_t * const        p_index,
                        field_encoder_handler_t fp_field_encoder)
{
    uint32_t err_code = NRF_SUCCESS;

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    p_buf[*p_index] = (p_field == NULL) ? SER_FIELD_NOT_PRESENT : SER_FIELD_PRESENT;
    *p_index       += 1;

    if (p_field && (fp_field_encoder != NULL))
    {
        err_code = fp_field_encoder(p_field, p_buf, buf_len, p_index);
    }

    return err_code;
}


uint32_t cond_field_dec(uint8_t const * const   p_buf,
                        uint32_t                buf_len,
                        uint32_t * const        p_index,
                        void * * const          pp_field,
                        field_decoder_handler_t fp_field_parser)
{
    uint32_t err_code = NRF_SUCCESS;
    uint8_t  is_present;

    SER_ASSERT_LENGTH_LEQ(1, buf_len - *p_index);
    uint8_dec(p_buf, buf_len, p_index, &is_present);

    if (is_present == SER_FIELD_PRESENT)
    {
        SER_ASSERT_NOT_NULL(pp_field);
        SER_ASSERT_NOT_NULL(*pp_field);

        if (fp_field_parser != NULL)
        {
            err_code = fp_field_parser(p_buf, buf_len, p_index, *pp_field);
        }
    }
    else if (is_present == SER_FIELD_NOT_PRESENT)
    {
        if (pp_field != NULL)
        {
            *pp_field = NULL;
        }
    }
    else
    {
        err_code = NRF_ERROR_INVALID_DATA;
    }

    return err_code;
}
