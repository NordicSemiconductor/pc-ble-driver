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

#include "slip.h"
#include "nrf_error.h"
#include "sd_rpc_types.h"
#include <cstddef>
#include <vector>

constexpr uint8_t SLIP_END     = 0xC0;
constexpr uint8_t SLIP_ESC     = 0xDB;
constexpr uint8_t SLIP_ESC_END = 0xDC;
constexpr uint8_t SLIP_ESC_ESC = 0xDD;

void slip_encode(const std::vector<uint8_t> &in_packet, std::vector<uint8_t> &out_packet)
{
    out_packet.push_back(SLIP_END);

    for (auto i : in_packet)
    {
        if (i == SLIP_END)
        {
            out_packet.push_back(SLIP_ESC);
            out_packet.push_back(SLIP_ESC_END);
        }
        else if (i == SLIP_ESC)
        {
            out_packet.push_back(SLIP_ESC);
            out_packet.push_back(SLIP_ESC_ESC);
        }
        else
        {
            out_packet.push_back(i);
        }
    }

    out_packet.push_back(SLIP_END);
}

uint32_t slip_decode(const std::vector<uint8_t> &packet, std::vector<uint8_t> &out_packet)
{
    for (std::size_t i = 0; i < packet.size(); i++)
    {
        if (packet[i] == SLIP_END)
        {
            continue;
        }
        else if (packet[i] == SLIP_ESC)
        {
            i++;
            if (i == packet.size())
            {
                return NRF_ERROR_SD_RPC_H5_TRANSPORT_SLIP_DECODING;
            }

            if (packet[i] == SLIP_ESC_END)
            {
                out_packet.push_back(SLIP_END);
            }
            else if (packet[i] == SLIP_ESC_ESC)
            {
                out_packet.push_back(SLIP_ESC);
            }
            else
            {
                return NRF_ERROR_SD_RPC_H5_TRANSPORT_SLIP_DECODING;
            }
        }
        else
        {
            out_packet.push_back(packet[i]);
        }
    }

    return NRF_SUCCESS;
}
