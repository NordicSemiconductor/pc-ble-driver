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
