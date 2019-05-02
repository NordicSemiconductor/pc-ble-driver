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

#include "sd_rpc.h"

#include "adapter_internal.h"
#include "ble_common.h"
#include "h5_transport.h"
#include "serial_port_enum.h"
#include "serialization_transport.h"
#include "uart_transport.h"
#include "uart_settings_boost.h"
#include "app_ble_gap.h"

#include <cstdlib>

uint32_t sd_rpc_serial_port_enum(sd_rpc_serial_port_desc_t serial_port_descs[], uint32_t *size)
{
    if (size == nullptr)
    {
        return NRF_ERROR_NULL;
    }

    const std::list<SerialPortDesc> &descs = EnumSerialPorts();

    if (descs.size() > *size)
    {
        return NRF_ERROR_DATA_SIZE;
    }

    *size = static_cast<uint32_t>(descs.size());

    auto i = 0;
    for (auto &desc : descs)
    {
        strncpy(serial_port_descs[i].port, desc.comName.c_str(), SD_RPC_MAXPATHLEN);
        strncpy(serial_port_descs[i].manufacturer, desc.manufacturer.c_str(), SD_RPC_MAXPATHLEN);
        strncpy(serial_port_descs[i].serialNumber, desc.serialNumber.c_str(), SD_RPC_MAXPATHLEN);
        strncpy(serial_port_descs[i].pnpId, desc.pnpId.c_str(), SD_RPC_MAXPATHLEN);
        strncpy(serial_port_descs[i].locationId, desc.locationId.c_str(), SD_RPC_MAXPATHLEN);
        strncpy(serial_port_descs[i].vendorId, desc.vendorId.c_str(), SD_RPC_MAXPATHLEN);
        strncpy(serial_port_descs[i].productId, desc.productId.c_str(), SD_RPC_MAXPATHLEN);
        ++i;
    }

    return NRF_SUCCESS;
}

physical_layer_t *sd_rpc_physical_layer_create_uart(const char *port_name, uint32_t baud_rate,
                                                    sd_rpc_flow_control_t flow_control,
                                                    sd_rpc_parity_t parity)
{
    const auto physicalLayer = static_cast<physical_layer_t *>(malloc(sizeof(physical_layer_t)));

    UartCommunicationParameters uartSettings = {};
    uartSettings.portName                    = port_name;
    uartSettings.baudRate                    = baud_rate;

    if (flow_control == SD_RPC_FLOW_CONTROL_NONE)
    {
        uartSettings.flowControl = UartFlowControlNone;
    }
    else if (flow_control == SD_RPC_FLOW_CONTROL_HARDWARE)
    {
        uartSettings.flowControl = UartFlowControlHardware;
    }

    if (parity == SD_RPC_PARITY_NONE)
    {
        uartSettings.parity = UartParityNone;
    }
    else if (parity == SD_RPC_PARITY_EVEN)
    {
        uartSettings.parity = UartParityEven;
    }

    uartSettings.stopBits = UartStopBitsOne;
    uartSettings.dataBits = UartDataBitsEight;

    const auto uart         = new UartTransport(uartSettings);
    physicalLayer->internal = static_cast<void *>(uart);
    return physicalLayer;
}

data_link_layer_t *sd_rpc_data_link_layer_create_bt_three_wire(physical_layer_t *physical_layer,
                                                               uint32_t retransmission_interval)
{
    const auto dataLinkLayer = static_cast<data_link_layer_t *>(malloc(sizeof(data_link_layer_t)));
    const auto physicalLayer = static_cast<UartTransport *>(physical_layer->internal);
    const auto h5            = new H5Transport(physicalLayer, retransmission_interval);
    dataLinkLayer->internal  = static_cast<void *>(h5);
    return dataLinkLayer;
}

transport_layer_t *sd_rpc_transport_layer_create(data_link_layer_t *data_link_layer,
                                                 uint32_t response_timeout)
{
    const auto transportLayer = static_cast<transport_layer_t *>(malloc(sizeof(transport_layer_t)));
    const auto dataLinkLayer  = static_cast<H5Transport *>(data_link_layer->internal);
    const auto serialization  = new SerializationTransport(dataLinkLayer, response_timeout);
    transportLayer->internal  = serialization;
    return transportLayer;
}

adapter_t *sd_rpc_adapter_create(transport_layer_t *transport_layer)
{
    const auto adapterLayer   = static_cast<adapter_t *>(malloc(sizeof(adapter_t)));
    const auto transportLayer = static_cast<SerializationTransport *>(transport_layer->internal);
    const auto adapter        = new AdapterInternal(transportLayer);
    adapterLayer->internal    = static_cast<void *>(adapter);
    return adapterLayer;
}

void sd_rpc_adapter_delete(adapter_t *adapter)
{
    const auto adapterLayer = static_cast<AdapterInternal *>(adapter->internal);

    if (adapterLayer == nullptr)
    {
        return;
    }

    delete adapterLayer;
    adapter->internal = nullptr;
}

uint32_t sd_rpc_open(adapter_t *adapter, sd_rpc_status_handler_t status_handler,
                     sd_rpc_evt_handler_t event_handler, sd_rpc_log_handler_t log_handler)
{
    auto adapterLayer = static_cast<AdapterInternal *>(adapter->internal);

    if (adapterLayer == nullptr)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    const auto err_code = adapterLayer->open(status_handler, event_handler, log_handler);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Create a BLE GAP state object
    return app_ble_gap_state_create(adapterLayer->transport);
}

uint32_t sd_rpc_close(adapter_t *adapter)
{
    const auto adapterLayer = static_cast<AdapterInternal *>(adapter->internal);

    if (adapterLayer == nullptr)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    const auto err_code = adapterLayer->close();

    // Delete BLE GAP state object
    app_ble_gap_state_delete(adapterLayer->transport);

    return err_code;
}

uint32_t sd_rpc_log_handler_severity_filter_set(adapter_t *adapter,
                                                sd_rpc_log_severity_t severity_filter)
{
    auto adapterLayer = static_cast<AdapterInternal *>(adapter->internal);

    if (adapterLayer == nullptr)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    return adapterLayer->logSeverityFilterSet(severity_filter);
}

uint32_t sd_rpc_conn_reset(adapter_t *adapter, sd_rpc_reset_t reset_mode)
{
    auto adapterLayer = static_cast<AdapterInternal *>(adapter->internal);

    if (adapterLayer == nullptr)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    const uint32_t tx_buffer_length = 1; // This command has 1 byte payload, hence length 1
    std::vector<uint8_t> tx_buffer(tx_buffer_length);
    tx_buffer[0] = static_cast<uint8_t>(reset_mode);

    return adapterLayer->transport->send(tx_buffer, nullptr, SERIALIZATION_RESET_CMD);
}
