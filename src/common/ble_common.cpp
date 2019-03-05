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

#include "ble_common.h"

#include <memory>
#include <sstream>

#include "adapter_internal.h"
#include "app_ble_gap.h"
#include "nrf_error.h"
#include "ser_config.h"

// AdapterRequestReplyCodecContext

RequestReplyCodecContext::RequestReplyCodecContext(void *adapterId)
{
    app_ble_gap_set_current_adapter_id(adapterId, REQUEST_REPLY_CODEC_CONTEXT);
}

RequestReplyCodecContext::~RequestReplyCodecContext()
{
    app_ble_gap_unset_current_adapter_id(REQUEST_REPLY_CODEC_CONTEXT);
}

EventCodecContext::EventCodecContext(void *adapterId)
{
    app_ble_gap_set_current_adapter_id(adapterId, EVENT_CODEC_CONTEXT);
}

EventCodecContext::~EventCodecContext()
{
    app_ble_gap_unset_current_adapter_id(EVENT_CODEC_CONTEXT);
}

uint32_t encode_decode(adapter_t *adapter, const encode_function_t &encode_function,
                       const decode_function_t &decode_function)
{
    std::stringstream error_message;
    auto _adapter = static_cast<AdapterInternal *>(adapter->internal);

    // Create rx_buffer
    std::shared_ptr<std::vector<uint8_t>> rx_buffer;
    
    if (decode_function)
    {
        rx_buffer = std::make_shared<std::vector<uint8_t>>(SER_HAL_TRANSPORT_MAX_PKT_SIZE);
    }

    // Create tx_buffer
    uint32_t tx_buffer_length = SER_HAL_TRANSPORT_MAX_PKT_SIZE;
    std::vector<uint8_t> tx_buffer(tx_buffer_length);
    auto err_code = encode_function(tx_buffer.data(), &tx_buffer_length);
    tx_buffer.resize(tx_buffer_length);

    if (AdapterInternal::isInternalError(err_code))
    {
        error_message << "Not able to encode packet. Code: 0x" << std::hex << err_code;
        _adapter->statusHandler(PKT_ENCODE_ERROR, error_message.str());
        return NRF_ERROR_SD_RPC_ENCODE;
    }

    err_code = _adapter->transport->send(tx_buffer, rx_buffer);

    if (AdapterInternal::isInternalError(err_code))
    {
        error_message << "Error sending packet to target. Code: 0x" << std::hex << err_code;
        _adapter->statusHandler(PKT_SEND_ERROR, error_message.str());

        switch (err_code)
        {
            case NRF_ERROR_SD_RPC_H5_TRANSPORT_NO_RESPONSE:
                return NRF_ERROR_SD_RPC_NO_RESPONSE;
            case NRF_ERROR_SD_RPC_H5_TRANSPORT_STATE:
                return NRF_ERROR_SD_RPC_INVALID_STATE;
            default:
                return NRF_ERROR_SD_RPC_SEND;
        }
    }

    uint32_t result_code = NRF_SUCCESS;

    if (decode_function)
    {
        err_code = decode_function(rx_buffer->data(), static_cast<uint32_t>(rx_buffer->size()),
                                   &result_code);
    }

    if (AdapterInternal::isInternalError(err_code))
    {
        error_message << "Not able to decode packet. Code 0x" << std::hex << err_code;
        _adapter->statusHandler(PKT_DECODE_ERROR, error_message.str());
        return NRF_ERROR_SD_RPC_DECODE;
    }

    return result_code;
}
