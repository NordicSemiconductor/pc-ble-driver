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

 #include "ble_common.h"

#include <memory>
#include <sstream>

#include "adapter_internal.h"
#include "nrf_error.h"
#include "ser_config.h"

uint32_t encode_decode(adapter_t *adapter, encode_function_t encode_function, decode_function_t decode_function)
{
    uint32_t tx_buffer_length = SER_HAL_TRANSPORT_MAX_PKT_SIZE;
    uint32_t rx_buffer_length = 0;

    std::unique_ptr<uint8_t> tx_buffer(static_cast<uint8_t*>(std::malloc(SER_HAL_TRANSPORT_MAX_PKT_SIZE)));
    std::unique_ptr<uint8_t> rx_buffer(static_cast<uint8_t*>(std::malloc(SER_HAL_TRANSPORT_MAX_PKT_SIZE)));

    std::stringstream error_message;

    auto _adapter = static_cast<AdapterInternal*>(adapter->internal);

    uint32_t err_code = encode_function(tx_buffer.get(), &tx_buffer_length);

    if (_adapter->isInternalError(err_code))
    {
        error_message << "Not able to decode packet received from target. Code #" << err_code;
        _adapter->statusHandler(PKT_DECODE_ERROR, error_message.str().c_str());
        return NRF_ERROR_INTERNAL;
    }

    if (decode_function != nullptr)
    {
        err_code = _adapter->transport->send(
            tx_buffer.get(),
            tx_buffer_length,
            rx_buffer.get(),
            &rx_buffer_length);
    }
    else
    {
        err_code = _adapter->transport->send(
            tx_buffer.get(),
            tx_buffer_length,
            nullptr,
            &rx_buffer_length);
    }

    if (_adapter->isInternalError(err_code))
    {
        error_message << "Error sending packet to target. Code #" << err_code;
        _adapter->statusHandler(PKT_SEND_ERROR, error_message.str().c_str());
        return NRF_ERROR_INTERNAL;
    }

    uint32_t result_code = NRF_SUCCESS;

    if (decode_function != nullptr)
    {
        err_code = decode_function(rx_buffer.get(), rx_buffer_length, &result_code);
    }

    if (_adapter->isInternalError(err_code))
    {
        error_message << "Not able to decode packet. Code #" << err_code;
        _adapter->statusHandler(PKT_DECODE_ERROR, error_message.str().c_str());
        return NRF_ERROR_INTERNAL;
    }

    return result_code;
}
