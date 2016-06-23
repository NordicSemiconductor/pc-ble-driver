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

#ifndef H5_H
#define H5_H

#include <stdint.h>
#include <vector>

const uint32_t H5_HEADER_LENGTH = 4;

typedef enum
{
    ACK_PACKET = 0,
    HCI_COMMAND_PACKET = 1,
    ACL_DATA_PACKET = 2,
    SYNC_DATA_PACKET = 3,
    HCI_EVENT_PACKET = 4,
    RESET_PACKET = 5,
    VENDOR_SPECIFIC_PACKET = 14,
    LINK_CONTROL_PACKET = 15
} h5_pkt_type_t;

typedef enum
{
    CONTROL_PKT_RESET,
    CONTROL_PKT_ACK,
    CONTROL_PKT_SYNC,
    CONTROL_PKT_SYNC_RESPONSE,
    CONTROL_PKT_SYNC_CONFIG,
    CONTROL_PKT_SYNC_CONFIG_RESPONSE,
} control_pkt_type;

void h5_encode(std::vector<uint8_t> &in_packet,
               std::vector<uint8_t> &out_packet,
               uint8_t seq_num,
               uint8_t ack_num,
               bool crc_present,
               bool reliable_packet,
               h5_pkt_type_t packet_type);

uint32_t h5_decode(std::vector<uint8_t> &slip_dec_packet,
	std::vector<uint8_t> &h5_dec_packet,
	uint8_t *seq_num,
	uint8_t *ack_num,
    bool *_data_integrity,
    uint16_t *_payload_length,
    uint8_t *_header_checksum,
	bool *reliable_packet,
	h5_pkt_type_t *packet_type);

#endif //H5_H
