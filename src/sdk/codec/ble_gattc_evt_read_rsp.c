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

#include "ble_gattc_evt_app.h"
#include <string.h>
#include "ble_serialization.h"
#include "app_util.h"


uint32_t ble_gattc_evt_read_rsp_dec(uint8_t const * const p_buf,
                                    uint32_t              packet_len,
                                    ble_evt_t * const     p_event,
                                    uint32_t * const      p_event_len)
{
    uint32_t index = 0;
    uint16_t tmp_attr_len;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);

    SER_ASSERT_LENGTH_LEQ(12, packet_len);

    tmp_attr_len = uint16_decode(&(p_buf[10]));

    uint32_t event_len = offsetof(ble_gattc_evt_t, params.read_rsp) +
            offsetof(ble_gattc_evt_read_rsp_t, data) + tmp_attr_len;

    if (p_event == NULL)
    {
        *p_event_len = event_len;
        return NRF_SUCCESS;
    }

    SER_ASSERT(event_len <= *p_event_len, NRF_ERROR_DATA_SIZE);

    p_event->header.evt_id  = BLE_GATTC_EVT_READ_RSP;
    p_event->header.evt_len = event_len;
    uint16_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.conn_handle));
    uint16_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.gatt_status));
    uint16_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.error_handle));
    uint16_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.params.read_rsp.handle));
    uint16_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.params.read_rsp.offset));
    uint16_dec(p_buf, packet_len, &index, &(p_event->evt.gattc_evt.params.read_rsp.len));

    SER_ASSERT_LENGTH_LEQ(index + tmp_attr_len, packet_len);

    if (tmp_attr_len > 0)
    {
        memcpy(&(p_event->evt.gattc_evt.params.read_rsp.data[0]), &(p_buf[index]), tmp_attr_len);
        index += tmp_attr_len;
    }

    SER_ASSERT_LENGTH_EQ(index, packet_len);
    *p_event_len = event_len;

    return NRF_SUCCESS;
}
