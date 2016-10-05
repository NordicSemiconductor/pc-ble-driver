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

#include "ble_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_struct_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "app_util.h"

uint32_t ble_opt_get_req_enc(uint32_t                opt_id,
                             ble_opt_t const * const p_opt,
                             uint8_t * const         p_buf,
                             uint32_t * const        p_buf_len)
{
    uint32_t index = 0;
    uint32_t err_code;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_buf_len);
    SER_ASSERT_LENGTH_LEQ(1+4+1, *p_buf_len);                       // [OPCODE][OP_ID][PRESENT]
    SER_ASSERT(((opt_id == BLE_COMMON_OPT_CONN_BW)         ||
                (opt_id == BLE_GAP_OPT_CH_MAP)             ||
                (opt_id == BLE_GAP_OPT_LOCAL_CONN_LATENCY) ||
                (opt_id == BLE_GAP_OPT_PASSKEY)            ||
                (opt_id == BLE_GAP_OPT_PRIVACY)            ||
                (opt_id == BLE_GAP_OPT_SCAN_REQ_REPORT)    ||
                (opt_id == BLE_GAP_OPT_COMPAT_MODE))  , NRF_ERROR_INVALID_PARAM);

    p_buf[index++] = SD_BLE_OPT_GET;

    err_code = uint32_t_enc(&opt_id, p_buf, *p_buf_len, &index);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    p_buf[index++] = (p_opt == NULL) ? SER_FIELD_NOT_PRESENT : SER_FIELD_PRESENT;

    *p_buf_len = index;

    return NRF_SUCCESS;
}

uint32_t ble_opt_get_rsp_dec(uint8_t const * const p_buf,
                             uint32_t              packet_len,
                             uint32_t      * const p_opt_id,
                             ble_opt_t     * const p_opt,
                             uint32_t      * const p_result_code)
{
    uint32_t index = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_opt_id);
    SER_ASSERT_NOT_NULL(p_opt);
    SER_ASSERT_NOT_NULL(p_result_code);

    uint32_t err_code = ser_ble_cmd_rsp_result_code_dec(p_buf, &index, packet_len,
                                                           SD_BLE_OPT_GET,
                                                           p_result_code);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if (*p_result_code != NRF_SUCCESS)
    {
        SER_ASSERT_LENGTH_EQ(index, packet_len);
        return NRF_SUCCESS;
    }

    (void) uint32_t_dec(p_buf, packet_len, &index, p_opt_id);
    SER_ASSERT(((*p_opt_id == BLE_COMMON_OPT_CONN_BW)         ||
                (*p_opt_id == BLE_GAP_OPT_CH_MAP)             ||
                (*p_opt_id == BLE_GAP_OPT_LOCAL_CONN_LATENCY) ||
                (*p_opt_id == BLE_GAP_OPT_PASSKEY)            ||
                (*p_opt_id == BLE_GAP_OPT_PRIVACY)            ||
                (*p_opt_id == BLE_GAP_OPT_SCAN_REQ_REPORT)    ||
                (*p_opt_id == BLE_GAP_OPT_COMPAT_MODE)), NRF_ERROR_INVALID_PARAM);

    switch (*p_opt_id)
    {
      case BLE_COMMON_OPT_CONN_BW:
          err_code = ble_common_opt_conn_bw_t_dec( p_buf, packet_len, &index, (void *)&(p_opt->common_opt.conn_bw));
      break;
      case BLE_GAP_OPT_CH_MAP:
          err_code = ble_gap_opt_ch_map_t_dec( p_buf, packet_len, &index, (void *)&(p_opt->gap_opt.ch_map));
      break;
      case BLE_GAP_OPT_LOCAL_CONN_LATENCY:
          err_code = ble_gap_opt_local_conn_latency_t_dec( p_buf, packet_len, &index, (void *)&(p_opt->gap_opt.local_conn_latency));
      break;
      case BLE_GAP_OPT_PASSKEY:
          err_code = ble_gap_opt_passkey_t_dec( p_buf, packet_len, &index, (void *)&(p_opt->gap_opt.passkey));
      break;
      case BLE_GAP_OPT_PRIVACY:
          err_code = ble_gap_opt_privacy_t_dec( p_buf, packet_len, &index, (void *)&(p_opt->gap_opt.privacy));
      break;
      case BLE_GAP_OPT_SCAN_REQ_REPORT:
          err_code = ble_gap_opt_scan_req_report_t_dec( p_buf, packet_len, &index, (void *)&(p_opt->gap_opt.scan_req_report));
      break;
      case BLE_GAP_OPT_COMPAT_MODE:
          err_code = ble_gap_opt_compat_mode_t_dec( p_buf, packet_len, &index, (void *)&(p_opt->gap_opt.compat_mode));
      break;
    }

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}


