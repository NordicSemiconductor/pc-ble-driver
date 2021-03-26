

#pragma once

#include <stdint.h>
#include <vector>

void slip_encode(const std::vector<uint8_t> &in_packet, std::vector<uint8_t> &out_packet);
uint32_t slip_decode(const std::vector<uint8_t> &packet, std::vector<uint8_t> &out_packet);
