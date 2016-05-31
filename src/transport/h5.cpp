/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
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

#include "h5.h"
#include "nrf_error.h"
#include <vector>
#include <algorithm>

const uint8_t seqNumMask = 0x07;
const uint8_t ackNumMask = 0x07;
const uint8_t ackNumPos = 3;
const uint8_t crcPresentMask = 0x01;
const uint8_t crcPresentPos = 6;
const uint8_t reliablePacketMask = 0x01;
const uint8_t reliablePacketPos = 7;

const uint8_t packetTypeMask = 0x0F;
const uint16_t payloadLengthFirstNibbleMask = 0x000F;
const uint16_t payloadLengthSecondNibbleMask = 0x0FF0;
const uint8_t payloadLengthOffset = 4;

uint8_t calculate_header_checksum(std::vector<uint8_t> &header)
{
    uint16_t checksum;

    checksum  = header[0];
    checksum += header[1];
    checksum += header[2];
    checksum &= 0xFFu;
    checksum  = (~checksum + 1u);

    return static_cast<uint8_t>(checksum);
}

uint16_t calculate_crc16_checksum(std::vector<uint8_t>::iterator start, std::vector<uint8_t>::iterator end)
{
    uint16_t crc = 0xFFFF;

    std::for_each(start, end, [&crc](uint8_t data) {
        crc = (crc >> 8) | (crc << 8);
        crc ^= data;
        crc ^= (crc & 0xFF) >> 4;
        crc ^= crc << 12;
        crc ^= (crc & 0xFF) << 5;
    });

    return crc;
}

void add_h5_header(std::vector<uint8_t> &out_packet,
                 uint8_t seq_num,
                 uint8_t ack_num,
                 bool crc_present,
                 bool reliable_packet,
                 uint8_t packet_type,
                 uint16_t payload_length)
{
    out_packet.push_back(
        (seq_num & seqNumMask)
        | ((ack_num & ackNumMask) << ackNumPos)
        | ((crc_present & crcPresentMask) << crcPresentPos)
        | ((reliable_packet & reliablePacketMask) << reliablePacketPos));

    out_packet.push_back(
        (packet_type & packetTypeMask)
        | ((payload_length & payloadLengthFirstNibbleMask) << payloadLengthOffset));

    out_packet.push_back((payload_length & payloadLengthSecondNibbleMask) >> payloadLengthOffset);
    out_packet.push_back(calculate_header_checksum(out_packet));
}

void add_crc16(std::vector<uint8_t> &out_packet)
{
    uint16_t crc16 = calculate_crc16_checksum(out_packet.begin(), out_packet.end());
    out_packet.push_back(crc16 & 0xFF);
    out_packet.push_back((crc16 >> 8) & 0xFF);
}

void h5_encode(std::vector<uint8_t> &in_packet,
               std::vector<uint8_t> &out_packet,
                   uint8_t seq_num,
                   uint8_t ack_num,
                   bool crc_present,
                   bool reliable_packet,
                   h5_pkt_type_t packet_type)
{
    add_h5_header(
        out_packet,
        seq_num,
        ack_num,
        crc_present,
        reliable_packet,
        packet_type,
        static_cast<uint16_t>(in_packet.size()));

    out_packet.insert(out_packet.end(), in_packet.begin(), in_packet.end());

    // Add CRC
    if (crc_present)
    {
        add_crc16(out_packet);
    }
}

uint32_t h5_decode(std::vector<uint8_t> &slipPayload,
                   std::vector<uint8_t> &h5Payload,
                   uint8_t *seq_num,
                   uint8_t *ack_num,
                   bool *_data_integrity,
                   uint16_t *_payload_length,
                   uint8_t *_header_checksum,
                   bool *reliable_packet,
                   h5_pkt_type_t *packet_type)
{
    uint16_t payload_length;

    if (slipPayload.size() < 4)
    {
        return NRF_ERROR_INVALID_LENGTH;
    }

    *seq_num = slipPayload[0] & seqNumMask;
    *ack_num = (slipPayload[0] >> ackNumPos) & ackNumMask;
    auto crc_present = static_cast<bool>(((slipPayload[0] >> crcPresentPos) & crcPresentMask) != 0);
    *reliable_packet = static_cast<bool>(((slipPayload[0] >> reliablePacketPos) & reliablePacketMask) != 0);
    *packet_type = static_cast<h5_pkt_type_t>(slipPayload[1] & packetTypeMask);
    payload_length = ((slipPayload[1] >> payloadLengthOffset) & payloadLengthFirstNibbleMask) + (static_cast<uint16_t>(slipPayload[2]) << payloadLengthOffset);
    auto header_checksum = slipPayload[3];

    // Check if received packet size matches the packet size stated in header
    auto calculatedPayloadSize = payload_length + H5_HEADER_LENGTH + (crc_present ? 2 : 0);

    if (slipPayload.size() != calculatedPayloadSize)
    {
        return NRF_ERROR_INVALID_DATA;
    }

    if (_payload_length != nullptr) *_payload_length = payload_length;
    if (_data_integrity != nullptr) *_data_integrity = crc_present;
    if (_header_checksum != nullptr) *_header_checksum = header_checksum;

    auto calculated_header_checksum = calculate_header_checksum(slipPayload);

    if (header_checksum != calculated_header_checksum)
    {
        return NRF_ERROR_INVALID_DATA;
    }

    if (crc_present)
    {
        uint16_t packet_checksum = slipPayload[payload_length + H5_HEADER_LENGTH] + (slipPayload[payload_length + H5_HEADER_LENGTH + 1] << 8);
        auto calculated_packet_checksum = calculate_crc16_checksum(slipPayload.begin(), slipPayload.begin() + payload_length + H5_HEADER_LENGTH);

        if (packet_checksum != calculated_packet_checksum)
        {
            return NRF_ERROR_INVALID_DATA;
        }
    }

    if (payload_length > 0)
    {
        auto payloadIterator = slipPayload.begin() + 4;
        h5Payload.insert(h5Payload.begin(), payloadIterator, payloadIterator + payload_length);
    }

    return NRF_SUCCESS;
}
