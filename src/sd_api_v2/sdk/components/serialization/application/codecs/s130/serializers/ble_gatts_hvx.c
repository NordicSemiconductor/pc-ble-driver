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

#include "ble_gatts_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "cond_field_serialization.h"
#include "ble_gap.h"
#include "app_util.h"


uint32_t ble_gatts_hvx_req_enc(uint16_t                             conn_handle,
                               ble_gatts_hvx_params_t const * const p_hvx_params,
                               uint8_t * const                      p_buf,
                               uint32_t * const                     p_buf_len)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);

    SER_ASSERT(p_hvx_params == NULL ||
               !(p_hvx_params->p_len == NULL && p_hvx_params->p_data != NULL), NRF_ERROR_NULL);

    SER_ASSERT_LENGTH_LEQ(index + 1 + 2 + 1 + 1, *p_buf_len);

    p_buf[index++] = SD_BLE_GATTS_HVX;
    index         += uint16_encode(conn_handle, &p_buf[index]);

    p_buf[index++] = (p_hvx_params != NULL) ? SER_FIELD_PRESENT : SER_FIELD_NOT_PRESENT;

    if (p_hvx_params != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(index + 2 + 1 + 2 + 2, *p_buf_len);
        index         += uint16_encode(p_hvx_params->handle, &p_buf[index]);
        p_buf[index++] = p_hvx_params->type;
        index         += uint16_encode(p_hvx_params->offset, &p_buf[index]);

        if (p_hvx_params->p_len != NULL)
        {
            SER_ASSERT_LENGTH_LEQ(index + 1 + 2 + 1, *p_buf_len);

            SER_ERROR_CHECK(*p_hvx_params->p_len <= BLE_GATTS_VAR_ATTR_LEN_MAX,
                            NRF_ERROR_INVALID_PARAM);
            p_buf[index++] = SER_FIELD_PRESENT;
            index         += uint16_encode(*(p_hvx_params->p_len), &p_buf[index]);

            if (p_hvx_params->p_data != NULL)
            {
                SER_ASSERT_LENGTH_LEQ(index + 1 + *(p_hvx_params->p_len), *p_buf_len);
                p_buf[index++] = SER_FIELD_PRESENT;
                memcpy(&(p_buf[index]), p_hvx_params->p_data, *(p_hvx_params->p_len));
                index += *(p_hvx_params->p_len);
            }
            else
            {
                p_buf[index++] = SER_FIELD_NOT_PRESENT;
            }
        }
        else
        {
            p_buf[index++] = SER_FIELD_NOT_PRESENT;
            p_buf[index++] = SER_FIELD_NOT_PRESENT;
        }
    }

    *p_buf_len = index;

    return NRF_SUCCESS;
}


uint32_t ble_gatts_hvx_rsp_dec(uint8_t const * const p_buf,
                               uint32_t              packet_len,
                               uint32_t * const      p_result_code,
                               uint16_t * * const    pp_bytes_written)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_result_code);

    uint32_t err_code;
    uint32_t index         = 0;
    uint32_t decode_result = ser_ble_cmd_rsp_result_code_dec(p_buf,
                                                             &index,
                                                             packet_len,
                                                             SD_BLE_GATTS_HVX,
                                                             p_result_code);

    if (decode_result != NRF_SUCCESS)
    {
        return decode_result;
    }

    if (*p_result_code != NRF_SUCCESS)
    {
        SER_ASSERT_LENGTH_EQ(index, packet_len);
        return NRF_SUCCESS;
    }

    err_code = cond_field_dec(p_buf, packet_len, &index, (void * *)pp_bytes_written, uint16_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return NRF_SUCCESS;
}
