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

#include <stdlib.h>
#include <string.h>
#include "ble_app.h"
#include "ble_serialization.h"
#include "ble_struct_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"


uint32_t ble_enable_req_enc(ble_enable_params_t * p_ble_enable_params,
                            uint8_t * const       p_buf,
                            uint32_t * const      p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_ENABLE);
    SER_PUSH_COND(p_ble_enable_params, ble_enable_params_t_enc);
    SER_REQ_ENC_END;
}


uint32_t ble_enable_rsp_dec(uint8_t const * const p_buf,
                            uint32_t              packet_len,
                            uint32_t * const      p_result_code)
{
    SER_RSP_DEC_RESULT_ONLY(SD_BLE_ENABLE);
}

uint32_t ble_opt_get_req_enc(uint32_t                opt_id,
                             ble_opt_t const * const p_opt,
                             uint8_t * const         p_buf,
                             uint32_t * const        p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_OPT_GET);

    SER_ASSERT(((opt_id == BLE_COMMON_OPT_CONN_BW)           ||
                (opt_id == BLE_COMMON_OPT_PA_LNA)            ||
                (opt_id == BLE_COMMON_OPT_CONN_EVT_EXT)      ||
                (opt_id == BLE_GAP_OPT_CH_MAP)               ||
                (opt_id == BLE_GAP_OPT_LOCAL_CONN_LATENCY)   ||
                (opt_id == BLE_GAP_OPT_PASSKEY)              ||
                (opt_id == BLE_GAP_OPT_SCAN_REQ_REPORT)      ||
                (opt_id == BLE_GAP_OPT_COMPAT_MODE)          ||
                (opt_id == BLE_GAP_OPT_AUTH_PAYLOAD_TIMEOUT) ||
                (opt_id == BLE_GAP_OPT_EXT_LEN)), NRF_ERROR_INVALID_PARAM);

    SER_PUSH_uint32(&opt_id);
    SER_PUSH_COND(p_opt, NULL);

    SER_REQ_ENC_END;
}

uint32_t ble_opt_get_rsp_dec(uint8_t const * const p_buf,
                             uint32_t              packet_len,
                             uint32_t      * const p_opt_id,
                             ble_opt_t     * const p_opt,
                             uint32_t      * const p_result_code)
{
    SER_RSP_DEC_BEGIN(SD_BLE_OPT_GET);

    SER_PULL_uint32(p_opt_id);
    SER_ASSERT(((*p_opt_id == BLE_COMMON_OPT_CONN_BW)           ||
                (*p_opt_id == BLE_COMMON_OPT_PA_LNA)            ||
                (*p_opt_id == BLE_COMMON_OPT_CONN_EVT_EXT)      ||
                (*p_opt_id == BLE_GAP_OPT_CH_MAP)               ||
                (*p_opt_id == BLE_GAP_OPT_LOCAL_CONN_LATENCY)   ||
                (*p_opt_id == BLE_GAP_OPT_PASSKEY)              ||
                (*p_opt_id == BLE_GAP_OPT_SCAN_REQ_REPORT)      ||
                (*p_opt_id == BLE_GAP_OPT_COMPAT_MODE)          ||
                (*p_opt_id == BLE_GAP_OPT_AUTH_PAYLOAD_TIMEOUT) ||
                (*p_opt_id == BLE_GAP_OPT_EXT_LEN)), NRF_ERROR_INVALID_PARAM);

    field_decoder_handler_t fp_decoder = NULL;
    void *                  p_struct = NULL;

    switch (*p_opt_id)
    {
        case BLE_COMMON_OPT_CONN_BW:
            fp_decoder = ble_common_opt_conn_bw_t_dec;
            p_struct   = &(p_opt->common_opt.conn_bw);
        break;
        case BLE_COMMON_OPT_PA_LNA:
            fp_decoder = ble_common_opt_pa_lna_t_dec;
            p_struct   = &(p_opt->common_opt.pa_lna);
        break;
        case BLE_COMMON_OPT_CONN_EVT_EXT:
            fp_decoder = ble_common_opt_conn_evt_ext_t_dec;
            p_struct   = &(p_opt->common_opt.conn_evt_ext);
        break;
        case BLE_GAP_OPT_CH_MAP:
            fp_decoder = ble_gap_opt_ch_map_t_dec;
            p_struct   =&(p_opt->gap_opt.ch_map);
        break;
        case BLE_GAP_OPT_LOCAL_CONN_LATENCY:
            fp_decoder = ble_gap_opt_local_conn_latency_t_dec;
            p_struct   = &(p_opt->gap_opt.local_conn_latency);
        break;
        case BLE_GAP_OPT_PASSKEY:
            fp_decoder = ble_gap_opt_passkey_t_dec;
            p_struct   = &(p_opt->gap_opt.passkey);
        break;
        case BLE_GAP_OPT_AUTH_PAYLOAD_TIMEOUT:
            fp_decoder = ble_gap_opt_auth_payload_timeout_t_dec;
            p_struct   = &(p_opt->gap_opt.auth_payload_timeout);
        break;
        case BLE_GAP_OPT_EXT_LEN:
            fp_decoder = ble_gap_opt_ext_len_t_dec;
            p_struct   = &(p_opt->gap_opt.ext_len);
        break;
        case BLE_GAP_OPT_SCAN_REQ_REPORT:
            fp_decoder = ble_gap_opt_scan_req_report_t_dec;
            p_struct   = &(p_opt->gap_opt.scan_req_report);
        break;
        case BLE_GAP_OPT_COMPAT_MODE:
            fp_decoder = ble_gap_opt_compat_mode_t_dec;
            p_struct   = &(p_opt->gap_opt.compat_mode);
        break;
        default:
            SER_ASSERT(NRF_ERROR_INVALID_PARAM, NRF_ERROR_INVALID_PARAM);
        break;
    }
    
    SER_PULL_FIELD(p_struct, fp_decoder);

    SER_RSP_DEC_END;
}



uint32_t ble_opt_set_req_enc(uint32_t const          opt_id,
                             ble_opt_t const * const p_opt,
                             uint8_t * const         p_buf,
                             uint32_t * const        p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_OPT_SET);

    SER_ASSERT(((opt_id == BLE_COMMON_OPT_CONN_BW)           ||
                (opt_id == BLE_COMMON_OPT_PA_LNA)            ||
                (opt_id == BLE_COMMON_OPT_CONN_EVT_EXT)      ||
                (opt_id == BLE_GAP_OPT_CH_MAP)               ||
                (opt_id == BLE_GAP_OPT_LOCAL_CONN_LATENCY)   ||
                (opt_id == BLE_GAP_OPT_PASSKEY)              ||
                (opt_id == BLE_GAP_OPT_SCAN_REQ_REPORT)      ||
                (opt_id == BLE_GAP_OPT_COMPAT_MODE)          ||
                (opt_id == BLE_GAP_OPT_AUTH_PAYLOAD_TIMEOUT) ||
                (opt_id == BLE_GAP_OPT_EXT_LEN)), NRF_ERROR_INVALID_PARAM);

    SER_PUSH_uint32(&opt_id);

    field_encoder_handler_t fp_encoder = NULL;
    void const *            p_struct = NULL;

    SER_PUSH_COND(p_opt, NULL);
    if (p_opt)
    {
        switch(opt_id)
        {
            case BLE_COMMON_OPT_CONN_BW:
                fp_encoder = ble_common_opt_conn_bw_t_enc;
                p_struct   = &(p_opt->common_opt.conn_bw);
            break;
            case BLE_COMMON_OPT_PA_LNA:
                fp_encoder = ble_common_opt_pa_lna_t_enc;
                p_struct   = &(p_opt->common_opt.pa_lna);
            break;
            case BLE_COMMON_OPT_CONN_EVT_EXT:
                fp_encoder = ble_common_opt_conn_evt_ext_t_enc;
                p_struct   = &(p_opt->common_opt.conn_evt_ext);
            break;
            case BLE_GAP_OPT_CH_MAP:
                fp_encoder = ble_gap_opt_ch_map_t_enc;
                p_struct   = &(p_opt->gap_opt.ch_map);
            break;
            case BLE_GAP_OPT_LOCAL_CONN_LATENCY:
                fp_encoder = ble_gap_opt_local_conn_latency_t_enc;
                p_struct   = &(p_opt->gap_opt.local_conn_latency);
            break;
            case BLE_GAP_OPT_PASSKEY:
                fp_encoder = ble_gap_opt_passkey_t_enc;
                p_struct   = &(p_opt->gap_opt.passkey);
            break;
            case BLE_GAP_OPT_AUTH_PAYLOAD_TIMEOUT:
                fp_encoder = ble_gap_opt_auth_payload_timeout_t_enc;
                p_struct   = &(p_opt->gap_opt.auth_payload_timeout);
            break;
            case BLE_GAP_OPT_EXT_LEN:
                fp_encoder = ble_gap_opt_ext_len_t_enc;
                p_struct   = &(p_opt->gap_opt.ext_len);
            break;
            case BLE_GAP_OPT_SCAN_REQ_REPORT:
                fp_encoder = ble_gap_opt_scan_req_report_t_enc;
                p_struct   = &(p_opt->gap_opt.scan_req_report);
            break;
            case BLE_GAP_OPT_COMPAT_MODE:
                fp_encoder = ble_gap_opt_compat_mode_t_enc;
                p_struct   = &(p_opt->gap_opt.compat_mode);
            break;
            default:
                SER_ASSERT(NRF_ERROR_INVALID_PARAM,NRF_ERROR_INVALID_PARAM);
            break;
        }

        SER_PUSH_FIELD(p_struct, fp_encoder);
    }

    SER_REQ_ENC_END;
}


uint32_t ble_opt_set_rsp_dec(uint8_t const * const p_buf,
                             uint32_t              packet_len,
                             uint32_t * const      p_result_code)
{
    SER_RSP_DEC_RESULT_ONLY(SD_BLE_OPT_SET);
}

uint32_t ble_tx_packet_count_get_req_enc(uint16_t              conn_handle,
                                         uint8_t const * const p_count,
                                         uint8_t * const       p_buf,
                                         uint32_t * const      p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_TX_PACKET_COUNT_GET);

    SER_PUSH_uint16(&conn_handle);
    SER_PUSH_COND(p_count, NULL);

    SER_REQ_ENC_END;
}


uint32_t ble_tx_packet_count_get_rsp_dec(uint8_t const * const p_buf,
                                         uint32_t              packet_len,
                                         uint8_t * * const     pp_count,
                                         uint32_t * const      p_result_code)
{
    SER_RSP_DEC_BEGIN(SD_BLE_TX_PACKET_COUNT_GET);
    SER_PULL_COND(pp_count, uint8_t_dec);
    SER_RSP_DEC_END;
}

uint32_t ble_user_mem_reply_req_enc(uint16_t                     conn_handle,
                                    ble_user_mem_block_t const * p_block,
                                    uint8_t * const              p_buf,
                                    uint32_t * const             p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_USER_MEM_REPLY);

    SER_PUSH_uint16(&conn_handle);
    SER_PUSH_COND(p_block, ble_user_mem_block_t_enc);

    SER_REQ_ENC_END;
}

uint32_t ble_user_mem_reply_rsp_dec(uint8_t const * const p_buf,
                                    uint32_t              packet_len,
                                    uint32_t      * const p_result_code)
{
    SER_RSP_DEC_RESULT_ONLY(SD_BLE_USER_MEM_REPLY);
}



uint32_t ble_uuid_decode_req_enc(uint8_t               uuid_le_len,
                                 uint8_t const * const p_uuid_le,
                                 ble_uuid_t * const    p_uuid,
                                 uint8_t * const       p_buf,
                                 uint32_t * const      p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_UUID_DECODE);
    
    SER_PUSH_len8data(p_uuid_le, uuid_le_len);
    SER_PUSH_COND(p_uuid, NULL);

    SER_REQ_ENC_END;
}


uint32_t ble_uuid_decode_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 ble_uuid_t * * const  pp_uuid,
                                 uint32_t * const      p_result_code)
{
    SER_RSP_DEC_BEGIN(SD_BLE_UUID_DECODE);
    SER_PULL_COND(pp_uuid, ble_uuid_t_dec);
    SER_RSP_DEC_END;
}


uint32_t ble_uuid_encode_req_enc(ble_uuid_t const * const p_uuid,
                                 uint8_t const * const    p_uuid_le_len,
                                 uint8_t const * const    p_uuid_le,
                                 uint8_t * const          p_buf,
                                 uint32_t * const         p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_UUID_ENCODE);

    SER_PUSH_COND(p_uuid, ble_uuid_t_enc);
    SER_PUSH_COND(p_uuid_le_len, NULL);
    SER_PUSH_COND(p_uuid_le, NULL);

    SER_REQ_ENC_END;
}


uint32_t ble_uuid_encode_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 uint8_t * const       p_uuid_le_len,
                                 uint8_t * const       p_uuid_le,
                                 uint32_t * const      p_result_code)
{
    SER_RSP_DEC_BEGIN(SD_BLE_UUID_ENCODE);

    uint8_t uuid_le_len;
    SER_PULL_uint8(&uuid_le_len);
    if (p_uuid_le_len)
    {
        *p_uuid_le_len = uuid_le_len;
        if (p_uuid_le)
        {
            SER_PULL_uint8array(p_uuid_le, uuid_le_len);
        }
    }

    SER_RSP_DEC_END;
}

uint32_t ble_uuid_vs_add_req_enc(ble_uuid128_t const * const p_vs_uuid,
                                 uint8_t * const             p_uuid_type,
                                 uint8_t * const             p_buf,
                                 uint32_t * const            p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_UUID_VS_ADD);

    SER_PUSH_COND(p_vs_uuid, ble_uuid128_t_enc);
    SER_PUSH_COND(p_uuid_type, NULL);

    SER_REQ_ENC_END;
}


uint32_t ble_uuid_vs_add_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 uint8_t * * const     pp_uuid_type,
                                 uint32_t * const      p_result_code)
{
    SER_RSP_DEC_BEGIN(SD_BLE_UUID_VS_ADD);
    
    SER_ASSERT_NOT_NULL(pp_uuid_type);
    SER_PULL_COND(pp_uuid_type, uint8_t_dec);

    SER_RSP_DEC_END;
}

uint32_t ble_version_get_req_enc(ble_version_t const * const p_version,
                                 uint8_t * const             p_buf,
                                 uint32_t * const            p_buf_len)
{
    SER_REQ_ENC_BEGIN(SD_BLE_VERSION_GET);
    SER_PUSH_COND(p_version, NULL);
    SER_REQ_ENC_END;
}

uint32_t ble_version_get_rsp_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 ble_version_t *       p_version,
                                 uint32_t * const      p_result_code)
{
    SER_RSP_DEC_BEGIN(SD_BLE_VERSION_GET);
    SER_PULL_FIELD(p_version, ble_version_t_dec);
    SER_RSP_DEC_END;
}

