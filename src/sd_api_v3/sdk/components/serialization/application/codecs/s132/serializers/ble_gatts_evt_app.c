/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
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

#include "ble_gatts_evt_app.h"
#include "ble_serialization.h"
#include "ble_gatts_struct_serialization.h"
#include "app_ble_user_mem.h"
#include "app_util.h"

extern ser_ble_user_mem_t m_app_user_mem_table[];

uint32_t ble_gatts_evt_hvc_dec(uint8_t const * const p_buf,
                               uint32_t              packet_len,
                               ble_evt_t * const     p_event,
                               uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTS_EVT_HVC, gatts, hvc);

    SER_PULL_uint16(&p_event->evt.gatts_evt.conn_handle);
    SER_PULL_FIELD(&p_event->evt.gatts_evt.params.hvc,
                     ble_gatts_evt_hvc_t_dec);

    SER_EVT_DEC_END;
}

uint32_t ble_gatts_evt_rw_authorize_request_dec(uint8_t const * const p_buf,
                                                uint32_t              packet_len,
                                                ble_evt_t * const     p_event,
                                                uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTS_EVT_HVC, gatts, rw_authorize_request);

    SER_PULL_uint16(&p_event->evt.gatts_evt.conn_handle);
    SER_PULL_FIELD_EXTENDED(&p_event->evt.gatts_evt.params.authorize_request, ble_gatts_evt_rw_authorize_request_t_dec);

    //Correct event length / memory sync.
    if (p_event->evt.gatts_evt.params.authorize_request.type == BLE_GATTS_AUTHORIZE_TYPE_READ)
    {
        evt_struct_len = offsetof(ble_evt_t, evt.gatts_evt.params.authorize_request.request.read)
                         - offsetof(ble_evt_t, evt)
                         + sizeof(ble_gatts_evt_read_t);
    }
    else if ((p_event->evt.gatts_evt.params.authorize_request.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE) &&
             ( (p_event->evt.gatts_evt.params.authorize_request.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
               (p_event->evt.gatts_evt.params.authorize_request.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)))
    {
        uint32_t conn_index;
        if(app_ble_user_mem_context_find(p_event->evt.gatts_evt.conn_handle, &conn_index) != NRF_ERROR_NOT_FOUND)
        {
            SER_PULL_len16data(&m_app_user_mem_table[conn_index].mem_block.p_mem, &m_app_user_mem_table[conn_index].mem_block.len);
        }
    }

    SER_EVT_DEC_END;
}


uint32_t ble_gatts_evt_sc_confirm_dec(uint8_t const * const p_buf,
                                      uint32_t              packet_len,
                                      ble_evt_t * const     p_event,
                                      uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN_NO_STRUCT(BLE_GATTS_EVT_SC_CONFIRM, gatts);
    SER_PULL_uint16(&p_event->evt.gatts_evt.conn_handle);
    SER_EVT_DEC_END;
}


uint32_t ble_gatts_evt_sys_attr_missing_dec(uint8_t const * const p_buf,
                                            uint32_t              packet_len,
                                            ble_evt_t * const     p_event,
                                            uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTS_EVT_SYS_ATTR_MISSING, gatts, sys_attr_missing);

    SER_PULL_uint16(&p_event->evt.gatts_evt.conn_handle);
    SER_PULL_FIELD(&p_event->evt.gatts_evt.params.sys_attr_missing,
                     ble_gatts_evt_sys_attr_missing_t_dec);

    SER_EVT_DEC_END;
}


uint32_t ble_gatts_evt_timeout_dec(uint8_t const * const p_buf,
                                   uint32_t              packet_len,
                                   ble_evt_t * const     p_event,
                                   uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTS_EVT_TIMEOUT, gatts, timeout);

    SER_PULL_uint16(&p_event->evt.gatts_evt.conn_handle);
    SER_PULL_FIELD(&p_event->evt.gatts_evt.params.timeout,
                     ble_gatts_evt_timeout_t_dec);

    SER_EVT_DEC_END;
}




uint32_t ble_gatts_evt_write_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 ble_evt_t * const     p_event,
                                 uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTS_EVT_WRITE, gatts, write);

    SER_PULL_uint16(&p_event->evt.gatts_evt.conn_handle);
    SER_PULL_FIELD_EXTENDED(&p_event->evt.gatts_evt.params.write, ble_gatts_evt_write_t_dec);

    if(p_event != NULL)
    {
        if(p_event->evt.gatts_evt.params.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW)
        {
            uint32_t conn_index;
            if(app_ble_user_mem_context_find(p_event->evt.gatts_evt.conn_handle, &conn_index) != NRF_ERROR_NOT_FOUND)
            {
                SER_PULL_len16data(&m_app_user_mem_table[conn_index].mem_block.p_mem, &m_app_user_mem_table[conn_index].mem_block.len);
                SER_ASSERT(err_code == NRF_SUCCESS, err_code);
            }
        }
    }

    SER_EVT_DEC_END;
}


uint32_t ble_gatts_evt_exchange_mtu_request_dec(uint8_t const * const p_buf,
                                                uint32_t              packet_len,
                                                ble_evt_t * const     p_event,
                                                uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST, gatts, exchange_mtu_request);

    SER_PULL_uint16(&p_event->evt.gatts_evt.conn_handle);
    SER_PULL_FIELD(&p_event->evt.gatts_evt.params.exchange_mtu_request, ble_gatts_evt_exchange_mtu_request_t_dec);

    SER_EVT_DEC_END;
}
