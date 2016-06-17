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

#include "app_util.h"
#include "ble.h"
#include "ble_serialization.h"
#include "ble_gattc_struct_serialization.h"
#include "ble_gattc_evt_app.h"


uint32_t ble_gattc_evt_char_val_by_uuid_read_rsp_dec(uint8_t const * const p_buf,
                                                     uint32_t              packet_len,
                                                     ble_evt_t * const     p_event,
                                                     uint32_t * const      p_event_len)
{
    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    uint32_t index = 0;
    uint32_t err_code;
    uint16_t conn_handle;
    uint16_t gatt_status;
    uint16_t error_handle;

    SER_ASSERT_LENGTH_LEQ(6, packet_len - index);

    uint32_t in_event_len = *p_event_len;

    *p_event_len = (offsetof(ble_evt_t, evt.gattc_evt.params)) - sizeof (ble_evt_hdr_t);

    uint16_dec(p_buf, packet_len, &index, &conn_handle);
    uint16_dec(p_buf, packet_len, &index, &gatt_status);
    uint16_dec(p_buf, packet_len, &index, &error_handle);

    void * p_data = NULL;

    if (p_event)
    {
        SER_ASSERT_LENGTH_LEQ(*p_event_len, in_event_len);

        p_event->header.evt_id              = BLE_GATTC_EVT_CHAR_VAL_BY_UUID_READ_RSP;
        p_event->evt.gattc_evt.conn_handle  = conn_handle;
        p_event->evt.gattc_evt.gatt_status  = gatt_status;
        p_event->evt.gattc_evt.error_handle = error_handle;

        p_data = &p_event->evt.gattc_evt.params.char_val_by_uuid_read_rsp;
    }
    else
    {
        p_data = NULL;
    }

    //call struct decoder with remaining size of event struct
    uint32_t temp_event_len = in_event_len - *p_event_len;
    err_code = ble_gattc_evt_char_val_by_uuid_read_rsp_t_dec(p_buf, packet_len, &index,
                                                             &temp_event_len, p_data);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    //update event length with the amount processed by struct decoder
    *p_event_len += temp_event_len;

    if (p_event)
    {
        p_event->header.evt_len = *p_event_len;
    }
    SER_ASSERT_LENGTH_EQ(index, packet_len);

    return err_code;
}
