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

#ifndef SLIP_H
#define SLIP_H

#include <stdint.h>
#include <vector>

void slip_encode(std::vector<uint8_t> &in_packet, std::vector<uint8_t> &out_packet);
uint32_t slip_decode(std::vector<uint8_t> &packet, std::vector<uint8_t> &out_packet);

#endif
