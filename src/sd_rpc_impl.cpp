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

#include "sd_rpc.h"

#include "adapter_internal.h"
#include "serialization_transport.h"
#include "h5_transport.h"
#include "uart_boost.h"
#include "uart_settings_boost.h"
#include "serial_port_enum.h"

#include <stdlib.h>

uint32_t sd_rpc_serial_port_enum(sdp_rpc_serial_port_desc_t serial_port_descs[], uint32_t size)
{
    std::list<SerialPortDesc*> descs;
    uint32_t ret;

    ret = EnumSerialPorts(descs);

    if(ret != NRF_SUCCESS)
    {
        return ret; 
    }

    return NRF_SUCCESS;
}

physical_layer_t *sd_rpc_physical_layer_create_uart(const char * port_name, uint32_t baud_rate, sd_rpc_flow_control_t flow_control, sd_rpc_parity_t parity)
{
    auto physicalLayer = static_cast<physical_layer_t *>(malloc(sizeof(physical_layer_t)));

    UartCommunicationParameters uartSettings;
    uartSettings.portName = port_name;
    uartSettings.baudRate = baud_rate;

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

    auto uart = new UartBoost(uartSettings);
    physicalLayer->internal = static_cast<void *>(uart);
    return physicalLayer;
}

data_link_layer_t *sd_rpc_data_link_layer_create_bt_three_wire(physical_layer_t *physical_layer, uint32_t retransmission_interval)
{
    auto dataLinkLayer = static_cast<data_link_layer_t *>(malloc(sizeof(data_link_layer_t)));
    auto physicalLayer = static_cast<Transport *>(physical_layer->internal);
    auto h5 = new H5Transport(physicalLayer, retransmission_interval);
    dataLinkLayer->internal = static_cast<void *>(h5);
    return dataLinkLayer;
}

transport_layer_t *sd_rpc_transport_layer_create(data_link_layer_t *data_link_layer, uint32_t response_timeout)
{
    auto transportLayer = static_cast<transport_layer_t *>(malloc(sizeof(transport_layer_t)));
    auto dataLinkLayer = static_cast<Transport *>(data_link_layer->internal);
    auto serialization = new SerializationTransport(dataLinkLayer, response_timeout);
    transportLayer->internal = serialization;
    return transportLayer;
}

adapter_t *sd_rpc_adapter_create(transport_layer_t* transport_layer)
{
    auto adapterLayer = static_cast<adapter_t *>(malloc(sizeof(adapter_t)));
    auto transportLayer = static_cast<SerializationTransport *>(transport_layer->internal);
    auto adapter = new AdapterInternal(transportLayer);
    adapterLayer->internal = static_cast<void *>(adapter);
    return adapterLayer;
}

void sd_rpc_adapter_delete(adapter_t *adapter)
{
    auto adapterLayer = static_cast<AdapterInternal *>(adapter->internal);
    delete adapterLayer;
}

uint32_t sd_rpc_open(adapter_t *adapter, sd_rpc_status_handler_t status_handler, sd_rpc_evt_handler_t event_handler, sd_rpc_log_handler_t log_handler)
{
    auto adapterLayer = static_cast<AdapterInternal *>(adapter->internal);
    return adapterLayer->open(status_handler, event_handler, log_handler);
}

uint32_t sd_rpc_close(adapter_t *adapter)
{
    auto adapterLayer = static_cast<AdapterInternal*>(adapter->internal);
    return adapterLayer->close();
}
