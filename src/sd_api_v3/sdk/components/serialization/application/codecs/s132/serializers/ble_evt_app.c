/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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

#include "ble_serialization.h"
#include "ble_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_util.h"
#include "ble_evt_app.h"
#include "app_ble_user_mem.h"


// Helper definitions for common event type names to be compliant with
// event serialization macros.
#define ble_common_evt_tx_complete_t         ble_evt_tx_complete_t
#define ble_common_evt_user_mem_request_t    ble_evt_user_mem_request_t
#define ble_common_evt_user_mem_release_t    ble_evt_user_mem_release_t
#define ble_common_evt_data_length_changed_t ble_evt_data_length_changed_t


extern ser_ble_user_mem_t m_app_user_mem_table[];

uint32_t ble_evt_user_mem_release_dec(uint8_t const * const p_buf,
                                      uint32_t              packet_len,
                                      ble_evt_t * const     p_event,
                                      uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_EVT_USER_MEM_RELEASE, common, user_mem_release);

    SER_PULL_uint16(&p_event->evt.common_evt.conn_handle);
    SER_PULL_uint8(&p_event->evt.common_evt.params.user_mem_release.type);
    SER_PULL_uint16(&p_event->evt.common_evt.params.user_mem_release.mem_block.len);

    //Set the memory pointer to not-null value.
    p_event->evt.common_evt.params.user_mem_release.mem_block.p_mem = (uint8_t *)~0;
    SER_PULL_COND(&p_event->evt.common_evt.params.user_mem_release.mem_block.p_mem, NULL);
    if (p_event->evt.common_evt.params.user_mem_release.mem_block.p_mem)
    {
        // Using connection handle find which mem block to release in Application Processor
        uint32_t user_mem_table_index;
        err_code = app_ble_user_mem_context_find(p_event->evt.common_evt.conn_handle, &user_mem_table_index);
        SER_ASSERT(err_code == NRF_SUCCESS, err_code);
        p_event->evt.common_evt.params.user_mem_release.mem_block.p_mem = 
            m_app_user_mem_table[user_mem_table_index].mem_block.p_mem;
    }

    // Now user memory context can be released
    err_code = app_ble_user_mem_context_destroy(p_event->evt.common_evt.conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    SER_EVT_DEC_END;
}

uint32_t ble_evt_tx_complete_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 ble_evt_t * const     p_event,
                                 uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_EVT_TX_COMPLETE, common, tx_complete);

    SER_PULL_uint16(&p_event->evt.common_evt.conn_handle);
    SER_PULL_uint8(&p_event->evt.common_evt.params.tx_complete.count);

    SER_EVT_DEC_END;
}


uint32_t ble_evt_user_mem_request_dec(uint8_t const * const p_buf,
                                      uint32_t              packet_len,
                                      ble_evt_t * const     p_event,
                                      uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_EVT_USER_MEM_REQUEST, common, user_mem_request);

    SER_PULL_uint16(&p_event->evt.common_evt.conn_handle);
    SER_PULL_uint8(&p_event->evt.common_evt.params.user_mem_request.type);

    SER_EVT_DEC_END;
}

uint32_t ble_evt_data_length_changed_dec(uint8_t const * const p_buf,
                                         uint32_t              packet_len,
                                         ble_evt_t * const     p_event,
                                         uint32_t * const      p_event_len)
{
    SER_EVT_DEC_BEGIN(BLE_EVT_DATA_LENGTH_CHANGED, common, data_length_changed);

    SER_PULL_uint16(&p_event->evt.common_evt.conn_handle);
    SER_PULL_FIELD(&p_event->evt.common_evt.params.data_length_changed, ble_evt_data_length_changed_t_dec);

    SER_EVT_DEC_END;
}
