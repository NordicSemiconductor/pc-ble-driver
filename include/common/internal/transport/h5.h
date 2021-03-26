#pragma once

#include <stdint.h>
#include <vector>

constexpr uint32_t H5_HEADER_LENGTH = 4;

enum class h5_pkt_type : uint8_t {
    ACK_PACKET             = 0,
    HCI_COMMAND_PACKET     = 1,
    ACL_DATA_PACKET        = 2,
    SYNC_DATA_PACKET       = 3,
    HCI_EVENT_PACKET       = 4,
    RESET_PACKET           = 5,
    VENDOR_SPECIFIC_PACKET = 14,
    LINK_CONTROL_PACKET    = 15
};

enum class control_pkt_type : uint8_t {
    RESET                = 0,
    ACK                  = 1,
    SYNC                 = 2,
    SYNC_RESPONSE        = 3,
    SYNC_CONFIG          = 4,
    SYNC_CONFIG_RESPONSE = 5,
    LAST                 = 10
};

void h5_encode(const std::vector<uint8_t> &in_packet, std::vector<uint8_t> &out_packet,
               uint8_t seq_num, uint8_t ack_num, bool crc_present, bool reliable_packet,
               h5_pkt_type packet_type);

uint32_t h5_decode(const std::vector<uint8_t> &slip_dec_packet, std::vector<uint8_t> &h5_dec_packet,
                   uint8_t *seq_num, uint8_t *ack_num, bool *_data_integrity,
                   uint16_t *_payload_length, uint8_t *_header_checksum, bool *reliable_packet,
                   h5_pkt_type *packet_type);
