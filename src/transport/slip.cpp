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

#include "slip.h"
#include "nrf_error.h"
#include <vector>
#include <algorithm>

#define SLIP_END 0xC0
#define SLIP_ESC 0xDB
#define SLIP_ESC_END 0xDC
#define SLIP_ESC_ESC 0xDD

void slip_encode(std::vector<uint8_t> &in_packet, std::vector<uint8_t> &out_packet)
{
    out_packet.push_back(SLIP_END);

    for (size_t i = 0; i < in_packet.size(); i++)
    {
        if (in_packet[i] == SLIP_END)
        {
            out_packet.push_back(SLIP_ESC);
            out_packet.push_back(SLIP_ESC_END);
        }
        else if (in_packet[i] == SLIP_ESC)
        {
            out_packet.push_back(SLIP_ESC);
            out_packet.push_back(SLIP_ESC_ESC);
        }
        else
        {
            out_packet.push_back(in_packet[i]);
        }
    }

    out_packet.push_back(SLIP_END);
}

uint32_t slip_decode(std::vector<uint8_t> &packet, std::vector<uint8_t> &out_packet)
{
    for (size_t i = 0; i < packet.size(); i++)
    {
        if (packet[i] == SLIP_END)
        {
            continue;
        }
        else if (packet[i] == SLIP_ESC) {
            i++;
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
                return NRF_ERROR_INVALID_DATA;
            }
        }
        else
        {
            out_packet.push_back(packet[i]);
        }
    }

    return NRF_SUCCESS;
}
