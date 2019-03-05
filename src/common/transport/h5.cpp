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

#include "h5.h"
#include "nrf_error.h"
#include "sd_rpc_types.h"
#include <algorithm>
#include <vector>

const uint8_t seqNumMask         = 0x07;
const uint8_t ackNumMask         = 0x07;
const uint8_t ackNumPos          = 3;
const uint8_t crcPresentMask     = 0x01;
const uint8_t crcPresentPos      = 6;
const uint8_t reliablePacketMask = 0x01;
const uint8_t reliablePacketPos  = 7;

const uint8_t packetTypeMask                 = 0x0F;
const uint16_t payloadLengthFirstNibbleMask  = 0x000F;
const uint16_t payloadLengthSecondNibbleMask = 0x0FF0;
const uint8_t payloadLengthOffset            = 4;

uint8_t calculate_header_checksum(const std::vector<uint8_t> &header)
{
    uint16_t checksum = header[0];
    checksum += header[1];
    checksum += header[2];
    checksum &= 0xFFu;
    checksum = (~checksum + 1u);

    return static_cast<uint8_t>(checksum);
}

uint16_t calculate_crc16_checksum(const std::vector<uint8_t>::const_iterator &start,
                                  const std::vector<uint8_t>::const_iterator &end)
{
    uint16_t crc = 0xFFFF;

    std::for_each(start, end, [&crc](const uint8_t data) {
        crc = (crc >> 8) | (crc << 8);
        crc ^= data;
        crc ^= (crc & 0xFF) >> 4;
        crc ^= crc << 12;
        crc ^= (crc & 0xFF) << 5;
    });

    return crc;
}

void add_h5_header(std::vector<uint8_t> &out_packet, const uint8_t seq_num, const uint8_t ack_num,
                   const bool crc_present, const bool reliable_packet, const uint8_t packet_type,
                   const uint16_t payload_length)
{
    out_packet.push_back((seq_num & seqNumMask) | ((ack_num & ackNumMask) << ackNumPos) |
                         ((crc_present & crcPresentMask) << crcPresentPos) |
                         ((reliable_packet & reliablePacketMask) << reliablePacketPos));

    out_packet.push_back((packet_type & packetTypeMask) |
                         ((payload_length & payloadLengthFirstNibbleMask) << payloadLengthOffset));

    out_packet.push_back((payload_length & payloadLengthSecondNibbleMask) >> payloadLengthOffset);
    out_packet.push_back(calculate_header_checksum(out_packet));
}

void add_crc16(std::vector<uint8_t> &out_packet)
{
    const auto crc16 = calculate_crc16_checksum(out_packet.begin(), out_packet.end());
    out_packet.push_back(crc16 & 0xFF);
    out_packet.push_back((crc16 >> 8) & 0xFF);
}

void h5_encode(const std::vector<uint8_t> &in_packet, std::vector<uint8_t> &out_packet,
               const uint8_t seq_num, const uint8_t ack_num, const bool crc_present,
               const bool reliable_packet, const h5_pkt_type_t packet_type)
{
    add_h5_header(out_packet, seq_num, ack_num, crc_present, reliable_packet, packet_type,
                  static_cast<uint16_t>(in_packet.size()));

    out_packet.insert(out_packet.end(), in_packet.begin(), in_packet.end());

    // Add CRC
    if (crc_present)
    {
        add_crc16(out_packet);
    }
}

uint32_t h5_decode(const std::vector<uint8_t> &slipPayload, std::vector<uint8_t> &h5Payload,
                   uint8_t *seq_num, uint8_t *ack_num, bool *_data_integrity,
                   uint16_t *_payload_length, uint8_t *_header_checksum, bool *reliable_packet,
                   h5_pkt_type_t *packet_type)
{
    if (slipPayload.size() < 4)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_SLIP_PAYLOAD_SIZE;
    }

    *seq_num = slipPayload[0] & seqNumMask;
    *ack_num = (slipPayload[0] >> ackNumPos) & ackNumMask;
    const auto crc_present =
        static_cast<bool>(((slipPayload[0] >> crcPresentPos) & crcPresentMask) != 0);
    *reliable_packet =
        static_cast<bool>(((slipPayload[0] >> reliablePacketPos) & reliablePacketMask) != 0);
    *packet_type = static_cast<h5_pkt_type_t>(slipPayload[1] & packetTypeMask);
    const uint16_t payload_length =
        ((slipPayload[1] >> payloadLengthOffset) & payloadLengthFirstNibbleMask) +
        (static_cast<uint16_t>(slipPayload[2]) << payloadLengthOffset);
    const auto header_checksum = slipPayload[3];

    // Check if received packet size matches the packet size stated in header
    const auto calculatedPayloadSize = payload_length + H5_HEADER_LENGTH + (crc_present ? 2 : 0);

    if (slipPayload.size() != calculatedPayloadSize)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_SLIP_CALCULATED_PAYLOAD_SIZE;
    }

    if (_payload_length != nullptr)
        *_payload_length = payload_length;
    if (_data_integrity != nullptr)
        *_data_integrity = crc_present;
    if (_header_checksum != nullptr)
        *_header_checksum = header_checksum;

    const auto calculated_header_checksum = calculate_header_checksum(slipPayload);

    if (header_checksum != calculated_header_checksum)
    {
        return NRF_ERROR_SD_RPC_H5_TRANSPORT_HEADER_CHECKSUM;
    }

    if (crc_present)
    {
        const uint16_t packet_checksum = slipPayload[payload_length + H5_HEADER_LENGTH] +
                                         (slipPayload[payload_length + H5_HEADER_LENGTH + 1] << 8);
        const auto calculated_packet_checksum = calculate_crc16_checksum(
            slipPayload.begin(), slipPayload.begin() + payload_length + H5_HEADER_LENGTH);

        if (packet_checksum != calculated_packet_checksum)
        {
            return NRF_ERROR_SD_RPC_H5_TRANSPORT_PACKET_CHECKSUM;
        }
    }

    if (payload_length > 0)
    {
        const auto payloadIterator = slipPayload.begin() + 4;
        h5Payload.insert(h5Payload.begin(), payloadIterator, payloadIterator + payload_length);
    }

    return NRF_SUCCESS;
}
