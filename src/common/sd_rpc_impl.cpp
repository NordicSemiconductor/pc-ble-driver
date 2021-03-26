#include "sd_rpc.h"

#include "adapter_internal.h"
#include "app_ble_gap.h"
#include "ble_common.h"
#include "event_handler.h"
#include "h5_transport.h"
#include "log_helper.h"
#include "logger_sink.h"
#include "serial_port_enum.h"
#include "serialization_transport.h"
#include "uart_settings_boost.h"
#include "uart_transport.h"

#include <spdlog/spdlog.h>

#include <cstdlib>

// NOLINTNEXTLINE
static const std::shared_ptr<::LoggerSink<std::mutex>> *logger_sink;

/* Function for properly initializing the logger sink.
 * - Adds the sink to the non-thread safe logger->sinks only once.
 * - Keeps the logger sink alive "forever" to prevent static initialization fiasco. Otherwise, the
 *   sink may be working on a log message while being destructed at the same time.
 */
static void s_initialize_logger_sink(std::shared_ptr<EventHandler> &event_handler)
{
    if (logger_sink == nullptr)
    {
        auto logger               = getLogger();
        using nrf_ble_driver_sink = ::LoggerSink<std::mutex>;

        // NOLINTNEXTLINE
        logger_sink = new std::shared_ptr<nrf_ble_driver_sink>(new nrf_ble_driver_sink());

        /* Warning: logger->sinks is not thread safe. */
        logger->sinks().push_back(*logger_sink);
    }

    (*logger_sink)->addEventHandler(event_handler);
}

static void s_remove_event_handler_from_logger_sink(std::shared_ptr<EventHandler> &event_handler)
{
    // NOLINTNEXTLINE
    assert(logger_sink != nullptr);
    (*logger_sink)->removeEventHandler(event_handler);
}

physical_layer_t *const sd_rpc_physical_layer_create_uart(const char *port_name, uint32_t baud_rate,
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

data_link_layer_t *const
sd_rpc_data_link_layer_create_bt_three_wire(physical_layer_t *const physical_layer,
                                            uint32_t retransmission_interval)
{
    const auto dataLinkLayer = static_cast<data_link_layer_t *>(malloc(sizeof(data_link_layer_t)));
    const auto physicalLayer = static_cast<UartTransport *>(physical_layer->internal);
    const auto h5            = new H5Transport(physicalLayer, retransmission_interval);
    dataLinkLayer->internal  = static_cast<void *>(h5);
    return dataLinkLayer;
}

transport_layer_t *const sd_rpc_transport_layer_create(data_link_layer_t *data_link_layer,
                                                       uint32_t response_timeout)
{
    const auto transportLayer = static_cast<transport_layer_t *>(malloc(sizeof(transport_layer_t)));
    const auto dataLinkLayer  = static_cast<H5Transport *>(data_link_layer->internal);
    const auto serialization  = new SerializationTransport(dataLinkLayer, response_timeout);
    transportLayer->internal  = serialization;
    return transportLayer;
}

adapter_t *const sd_rpc_adapter_create(transport_layer_t *transport_layer)
{
    const auto adapterLayer   = static_cast<adapter_t *>(malloc(sizeof(adapter_t)));
    const auto transportLayer = static_cast<SerializationTransport *>(transport_layer->internal);
    const auto adapter        = new AdapterInternal(transportLayer);
    adapterLayer->internal    = static_cast<void *>(adapter);

    // NOLINTNEXTLINE
    auto ehandler = std::make_shared<EventHandler>();
    s_initialize_logger_sink(ehandler);
    ehandler->start();

    return adapterLayer;
}

void sd_rpc_adapter_delete(adapter_t *const adapter)
{
    const auto adapterLayer = static_cast<AdapterInternal *>(adapter->internal);

    if (adapterLayer == nullptr)
    {
        return;
    }

    delete adapterLayer;
    adapter->internal = nullptr;
}

uint32_t sd_rpc_open(adapter_t *const adapter, sd_rpc_status_handler_t status_handler,
                     sd_rpc_evt_handler_t event_handler, sd_rpc_log_handler_t log_handler,
                     const void *const user_data_status, const void *const user_data_event,
                     const void *const user_data_log)
{
    auto adapterLayer = static_cast<AdapterInternal *>(adapter->internal);

    if (adapterLayer == nullptr)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    const auto err_code = adapterLayer->open(status_handler, event_handler, log_handler,
                                             user_data_status, user_data_event, user_data_log);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Create a BLE GAP state object
    return app_ble_gap_state_create(adapterLayer->transport);
}

uint32_t sd_rpc_close(adapter_t *const adapter)
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

uint32_t sd_rpc_log_handler_severity_filter_set(adapter_t *const adapter,
                                                sd_rpc_log_severity_t severity_filter)
{
    auto adapterLayer = static_cast<AdapterInternal *>(adapter->internal);

    if (adapterLayer == nullptr)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    return adapterLayer->logSeverityFilterSet(severity_filter);
}

uint32_t sd_rpc_conn_reset(adapter_t *const adapter, sd_rpc_reset_t reset_mode)
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
