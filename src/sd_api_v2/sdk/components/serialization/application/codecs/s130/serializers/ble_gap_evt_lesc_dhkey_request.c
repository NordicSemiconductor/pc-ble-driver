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

#include "ble_gap_evt_app.h"
#include "ble_serialization.h"
#include "ble_gap_struct_serialization.h"
#include "cond_field_serialization.h"
#include "app_ble_gap.h"

uint32_t ble_gap_evt_lesc_dhkey_request_dec(uint8_t const * const p_buf,
                                            uint32_t              packet_len,
                                            ble_evt_t * const     p_event,
                                            uint32_t * const      p_event_len)
{
    uint32_t index    = 0;

    SER_ASSERT_NOT_NULL(p_buf);
    SER_ASSERT_NOT_NULL(p_event_len);
    // [minimal packet is: 'conn_handle' + 'p_pk_peer' SER_FIELD_NOT_PRESENT +
    //  'oobd_req']
    SER_ASSERT_LENGTH_LEQ(sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t),
        packet_len);

    uint32_t event_len = offsetof (ble_gap_evt_t, params) + sizeof(uint8_t) + sizeof(uint8_t);

    if (p_event == NULL)
    {
        *p_event_len = event_len;
        return NRF_SUCCESS;
    }

    SER_ASSERT(event_len <= *p_event_len, NRF_ERROR_DATA_SIZE);

    p_event->header.evt_len = event_len;

    uint32_t err_code = uint16_t_dec(p_buf, packet_len, &index, &p_event->evt.gap_evt.conn_handle);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint32_t conn_index;
    err_code = app_ble_gap_sec_keys_find(p_event->evt.gap_evt.conn_handle, &conn_index);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    ble_gap_sec_keyset_t *keyset;
    err_code = app_ble_gap_sec_keys_get(conn_index, &keyset);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    p_event->evt.gap_evt.params.lesc_dhkey_request.p_pk_peer = keyset->keys_peer.p_pk;

    err_code = cond_field_dec(p_buf, packet_len, &index,
        (void **)&p_event->evt.gap_evt.params.lesc_dhkey_request.p_pk_peer, ble_gap_lesc_p256_pk_t_dec);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);

    uint8_t data;
    err_code = uint8_t_dec(p_buf, packet_len, &index, &data);
    SER_ASSERT(err_code == NRF_SUCCESS, err_code);
    p_event->evt.gap_evt.params.lesc_dhkey_request.oobd_req = data & 0x01;

    SER_ASSERT_LENGTH_EQ(index, packet_len);
    *p_event_len = event_len;

    return err_code;
}
