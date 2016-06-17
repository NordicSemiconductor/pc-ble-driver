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

#include "ble_gatts_evt_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "ble_gatts_struct_serialization.h"
#include "app_ble_user_mem.h"
#include "app_util.h"

extern ser_ble_user_mem_t m_app_user_mem_table[];

uint32_t ble_gatts_evt_write_dec(uint8_t const * const p_buf,
                                 uint32_t              packet_len,
                                 ble_evt_t * const     p_event,
                                 uint32_t * const      p_event_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    uint32_t err_code = NRF_SUCCESS;
    uint32_t index    = 0;

    uint32_t in_event_len = *p_event_len;

    *p_event_len = offsetof(ble_evt_t, evt.gatts_evt.params) - sizeof (ble_evt_hdr_t);

    uint16_t conn_handle;
    err_code = uint16_t_dec(p_buf, packet_len, &index, &conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    void * p_void_write = NULL;

    if (p_event != NULL)
    {
        SER_ASSERT_LENGTH_LEQ(*p_event_len, in_event_len);
        p_event->evt.gatts_evt.conn_handle = conn_handle;

        p_void_write = &(p_event->evt.gatts_evt.params.write);
    }

    uint32_t tmp_struct_len = in_event_len - *p_event_len;
    err_code = ble_gatts_evt_write_t_dec(p_buf,
                                         packet_len,
                                         &index,
                                         &tmp_struct_len,
                                         p_void_write);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    *p_event_len += tmp_struct_len;
    
    if(p_event != NULL)
    {
        if(p_event->evt.gatts_evt.params.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW)
        {
            uint32_t conn_index;

            if(app_ble_user_mem_context_find(p_event->evt.gatts_evt.conn_handle, &conn_index) != NRF_ERROR_NOT_FOUND)
            {        
                err_code = len16data_dec(p_buf, packet_len, &index, &m_app_user_mem_table[conn_index].mem_block.p_mem, &m_app_user_mem_table[conn_index].mem_block.len);
                SER_ASSERT(err_code == NRF_SUCCESS, err_code);
            }
        }
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}
