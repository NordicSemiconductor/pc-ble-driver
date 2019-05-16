#include "test_util_adapter_wrapper.h"

#if NRF_SD_BLE_API == 4 || NRF_SD_BLE_API > 6
#error "wrapper does not take into account this version of the SoftDevice API."
#endif

#include "test_util_conversion.h"
#include "test_util_role.h"

// Logging support
#include "logging.h"

#include "ble.h"
#include "sd_rpc.h"

#include "test_util_adapter_wrapper_scratchpad.h"

#include <cstring>
#include <map>
#include <test_util.h>
#include <thread>

constexpr uint8_t MaxPeripheralConnections    = 1;
constexpr uint8_t MaxCentralConnections       = 7;
constexpr uint8_t MaxCentralSecureConnections = 1;

namespace testutil {

uint16_t millisecondsToUnits(const double milliseconds, const sd_api_time_unit_t timeunit)
{
    const auto microseconds = milliseconds * 1000;
    return (static_cast<uint16_t>(microseconds / timeunit));
}

AdapterWrapper::AdapterWrapper(const Role &role, const std::string &port, const uint32_t baudRate,
                               const uint16_t mtu, const uint32_t retransmissionInterval,
                               uint32_t const responseTimeout)
    : address{}
    , m_role(role)
    , m_port(port)
    , m_async_error(false)
{
    m_adapter = adapterInit(port.c_str(), baudRate, retransmissionInterval, responseTimeout);

    AdapterWrapper::adapters[m_adapter->internal] = this;

#if NRF_SD_BLE_API > 5
    //
    // Create a fixed memory area to support SoftDevice API v6
    // advertising data change during advertising
    //
    m_data_buffers.resize(DATA_ROTATING_BUFFER_COUNT);
    for (auto &buffer : m_data_buffers)
    {
        buffer.reserve(ADV_DATA_BUFFER_SIZE);
    }
#endif

    // Setup scratchpad with default values
    setupScratchpad(mtu);
};

AdapterWrapper::~AdapterWrapper()
{
    if (m_adapter != nullptr)
    {
        const auto err_code = sd_rpc_close(m_adapter);
        get_logger()->debug("{} sd_rpc_close {}", role(), testutil::errorToString(err_code));

        // Remove adapter from map of adapters used in callbacks
        try
        {
            AdapterWrapper::adapters.erase(m_adapter->internal);
        }
        catch (const std::exception &e)
        {
            get_logger()->error("{} not possible to erase adapter in map of adapters, ", role(),
                                e.what());
        }

        sd_rpc_adapter_delete(m_adapter);
        get_logger()->debug("{} sd_rpc_adapter_delete called and returned.", role());
    }
}

uint32_t AdapterWrapper::configure()
{
    uint32_t err_code;

#if NRF_SD_BLE_API >= 5
    err_code = setBLECfg(scratchpad.config_id);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
#endif

    err_code = initBLEStack();

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

#if NRF_SD_BLE_API < 5
    err_code = setBLEOptions();

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
#endif

// Store the local address
#if NRF_SD_BLE_API >= 3
    err_code = sd_ble_gap_addr_get(m_adapter, &address);
    if (err_code != NRF_SUCCESS)
    {
        get_logger()->debug("{} sd_ble_gap_addr_get,", role(), testutil::errorToString(err_code));
    }
#else
    err_code = sd_ble_gap_address_get(m_adapter, &address);
    if (err_code != NRF_SUCCESS)
    {
        get_logger()->debug("{} sd_ble_gap_address_get failed, {}", role(),
                            testutil::errorToString(err_code));
    }
#endif

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    else
    {
        get_logger()->debug("{} GAP address is: {}", role(), testutil::asText(address));
    }

    return err_code;
}

uint32_t AdapterWrapper::connect(const ble_gap_addr_t *address)
{
    const auto err_code = sd_ble_gap_connect(m_adapter, address, &(scratchpad.scan_param),
                                             &(scratchpad.connection_param)
#if NRF_SD_BLE_API >= 5
                                                 ,
                                             scratchpad.config_id
#endif
    );

    if (err_code == NRF_SUCCESS)
    {
        scratchpad.connection_in_progress = true;
    }
    else
    {
        get_logger()->debug("{} sd_ble_gap_connect, {}", role(), testutil::errorToString(err_code));
    }

    return err_code;
}

bool AdapterWrapper::error() const
{
    return m_async_error;
}

uint32_t AdapterWrapper::open()
{
    return sd_rpc_open(m_adapter, statusHandler, eventHandler, logHandler);
}

uint32_t AdapterWrapper::close()
{
    return sd_rpc_close(m_adapter);
}

uint32_t AdapterWrapper::startScan(const bool resume, const bool extended, const bool incomplete)
{
#if NRF_SD_BLE_API == 6
    scratchpad.adv_report_receive_buffer.p_data  = scratchpad.adv_report_data_received;
    scratchpad.adv_report_receive_buffer.len     = sizeof(scratchpad.adv_report_data_received);
    scratchpad.scan_param.extended               = extended ? 1 : 0;
    scratchpad.scan_param.report_incomplete_evts = incomplete ? 1 : 0;
#endif

#if NRF_SD_BLE_API < 6
    if (resume == true)
    {
        get_logger()->debug("{} Not possible to resume a scan if SoftDevice API version < 6",
                            role());
        return NRF_ERROR_INVALID_STATE;
    }
#endif

    ble_gap_scan_params_t *scanParams;

    if (resume)
    {
        scanParams = nullptr;
    }
    else
    {
        scanParams = &scratchpad.scan_param;
    }

    const auto err_code = sd_ble_gap_scan_start(m_adapter, scanParams
#if NRF_SD_BLE_API == 6
                                                ,
                                                &(scratchpad.adv_report_receive_buffer)
#endif
    );

    if (err_code != NRF_SUCCESS)
    {
        if (!resume)
        {
            get_logger()->debug("{} Scan start failed with {}", role(),
                                testutil::errorToString(err_code));
        }
        else
        {
            get_logger()->debug("{} Scan resume failed with {}", role(),
                                testutil::errorToString(err_code));
        }
    }
    else
    {
        if (!resume)
        {
            get_logger()->debug("{} Scan started", role());
        }
        else
        {
            get_logger()->debug("{} Scan resumed", role());
        }
    }

    return err_code;
}

uint32_t AdapterWrapper::setupAdvertising(
    const std::vector<uint8_t> &advertisingData, const std::vector<uint8_t> &scanResponseData,
    const uint32_t interval, const uint32_t duration, const bool connectable, const bool extended,
    const bool scan_req_notification, const uint8_t set_id, const uint8_t primary_phy,
    const uint8_t secondary_phy, const uint8_t filter_policy, const uint32_t max_adv_events)
{
#if NRF_SD_BLE_API <= 5
    const uint8_t *sr_data       = nullptr;
    const uint8_t sr_data_length = 0;

    auto err_code = sd_ble_gap_adv_data_set(m_adapter, advertisingData.data(),
                                            static_cast<uint8_t>(advertisingData.size()), sr_data,
                                            sr_data_length);

    if (err_code != NRF_SUCCESS)
    {
        get_logger()->debug("{}  sd_ble_gap_adv_data_set, {}", role(),
                            testutil::errorToString(err_code));
        return err_code;
    }
#endif

#if NRF_SD_BLE_API == 6
    const auto advertisingDataSize = advertisingData.size();

    if (advertisingDataSize > testutil::ADV_DATA_BUFFER_SIZE)
    {
        get_logger()->debug("{} Advertising data is larger then the buffer set asize.", role());
        return NRF_ERROR_INVALID_PARAM;
    }

    if (advertisingDataSize == 0)
    {
        scratchpad.adv_report_adv_data.p_data = nullptr;
        scratchpad.adv_report_adv_data.len    = 0;
    }
    else
    {
        std::copy(advertisingData.begin(), advertisingData.end(),
                  scratchpad.adv_report_adv_data_buffer);
        scratchpad.adv_report_adv_data.p_data = scratchpad.adv_report_adv_data_buffer;
        scratchpad.adv_report_adv_data.len    = static_cast<uint16_t>(advertisingDataSize);
    }

    const auto scanResponseDataSize = scanResponseData.size();

    if (scanResponseDataSize == 0)
    {
        scratchpad.adv_report_scan_rsp_data.p_data = nullptr;
        scratchpad.adv_report_scan_rsp_data.len    = 0;
    }
    else
    {
        std::copy(scanResponseData.begin(), scanResponseData.end(),
                  scratchpad.adv_report_scan_rsp_data_buffer);
        scratchpad.adv_report_scan_rsp_data.p_data = scratchpad.adv_report_scan_rsp_data_buffer;
        scratchpad.adv_report_scan_rsp_data.len    = static_cast<uint16_t>(scanResponseDataSize);
    }

    // Tie together the advertisement setup
    scratchpad.adv_report_data.adv_data               = scratchpad.adv_report_adv_data;
    scratchpad.adv_report_data.scan_rsp_data          = scratchpad.adv_report_scan_rsp_data;
    scratchpad.adv_params.properties.anonymous        = 0;
    scratchpad.adv_params.properties.include_tx_power = 0;

    scratchpad.adv_params.set_id                = set_id;
    scratchpad.adv_params.interval              = interval;
    scratchpad.adv_params.duration              = duration;
    scratchpad.adv_params.filter_policy         = filter_policy;
    scratchpad.adv_params.scan_req_notification = (scan_req_notification ? 1 : 0);
    scratchpad.adv_params.primary_phy           = primary_phy;
    scratchpad.adv_params.secondary_phy         = secondary_phy;
    scratchpad.adv_params.max_adv_evts          = max_adv_events;

    // Support only undirected advertisement for now
    if (extended)
    {
        if (connectable)
        {
            scratchpad.adv_params.properties.type =
                BLE_GAP_ADV_TYPE_EXTENDED_CONNECTABLE_NONSCANNABLE_UNDIRECTED;
        }
        else
        {
            scratchpad.adv_params.properties.type =
                BLE_GAP_ADV_TYPE_EXTENDED_NONCONNECTABLE_SCANNABLE_UNDIRECTED;
        }
    }
    else
    {
        if (connectable)
        {
            scratchpad.adv_params.properties.type =
                BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
        }
        else
        {
            scratchpad.adv_params.properties.type =
                BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED;
        }
    }

    const auto err_code =
        sd_ble_gap_adv_set_configure(m_adapter, &(scratchpad.adv_handle),
                                     &(scratchpad.adv_report_data), &(scratchpad.adv_params));
#endif

    if (err_code != NRF_SUCCESS)
    {
        get_logger()->debug("{} Setup of advertisement failed, {}", role(),
                            testutil::errorToString(err_code));
    }
    else
    {
        get_logger()->debug("{} Setting advertisement success.", role());
    }

    return err_code;
}

uint32_t AdapterWrapper::startAdvertising()
{
    if (role() != Role::Peripheral)
    {
        get_logger()->debug("{} Wrong role, must be peripheral to advertise.", role());
        return NRF_ERROR_INVALID_STATE;
    }

    uint32_t err_code;

#if NRF_SD_BLE_API <= 3
    err_code = sd_ble_gap_adv_start(m_adapter, &(scratchpad.adv_params));
#elif NRF_SD_BLE_API == 5
    err_code = sd_ble_gap_adv_start(m_adapter, &(scratchpad.adv_params), scratchpad.config_id);
#elif NRF_SD_BLE_API == 6
    err_code = sd_ble_gap_adv_start(m_adapter, scratchpad.adv_handle, scratchpad.config_id);
#endif

    if (err_code != NRF_SUCCESS)
    {
        get_logger()->debug("{} Failed to start advertising, sd_ble_gap_adv_start, {}", role(),
                            testutil::errorToString(err_code));
    }

    return err_code;
}

#if NRF_SD_BLE_API > 5
data_buffer &AdapterWrapper::nextDataBuffer()
{
    m_data_buffers_index++;
    m_data_buffers_index = m_data_buffers_index % DATA_ROTATING_BUFFER_COUNT;
    return m_data_buffers[m_data_buffers_index];
}
#endif

uint32_t AdapterWrapper::changeAdvertisingData(const std::vector<uint8_t> &advertisingData,
                                               const std::vector<uint8_t> &scanResponseData)
{
#if NRF_SD_BLE_API == 6
    const auto advertisingDataSize = advertisingData.size();

    if (advertisingDataSize > testutil::ADV_DATA_BUFFER_SIZE)
    {
        get_logger()->debug("{} Advertising data is larger then the buffer set aside.", role());
        return NRF_ERROR_INVALID_PARAM;
    }

    ble_data_t adv_data;

    if (advertisingDataSize == 0)
    {
        adv_data.p_data = nullptr;
        adv_data.len    = 0;
    }
    else
    {
        auto &adv_data_buffer = nextDataBuffer();

        adv_data_buffer.resize(advertisingDataSize);
        std::copy(advertisingData.begin(), advertisingData.end(), adv_data_buffer.begin());

        adv_data.p_data = adv_data_buffer.data();
        adv_data.len    = static_cast<uint16_t>(advertisingDataSize);
    }

    const auto scanResponseDataSize = scanResponseData.size();

    ble_data_t scan_rsp_data;

    if (scanResponseDataSize == 0)
    {
        scan_rsp_data.p_data = nullptr;
        scan_rsp_data.len    = 0;
    }
    else
    {
        auto &scan_rsp_data_buffer = nextDataBuffer();
        scan_rsp_data_buffer.resize(scanResponseDataSize);
        std::copy(scanResponseData.begin(), scanResponseData.end(), scan_rsp_data_buffer.begin());
        scan_rsp_data.p_data = scan_rsp_data_buffer.data();
        scan_rsp_data.len    = static_cast<uint16_t>(scanResponseDataSize);
    }

    // Tie together the advertisement setup
    ble_gap_adv_data_t advertising_data;

    advertising_data.adv_data      = adv_data;
    advertising_data.scan_rsp_data = scan_rsp_data;

    get_logger()->debug(
        "{} Changing advertisement data to instance with addresses adv_data.p_data: {} "
        "adv_data.len: {} scan_rsp.p_data: {} scan_rsp_data.len:{}",
        role(), static_cast<void *>(advertising_data.adv_data.p_data),
        static_cast<uint32_t>(adv_data.len),
        static_cast<void *>(advertising_data.scan_rsp_data.p_data),
        static_cast<uint32_t>(scan_rsp_data.len));

    // Support only undirected advertisement for now
    const auto err_code = sd_ble_gap_adv_set_configure(m_adapter, &(scratchpad.adv_handle),
                                                       &advertising_data, nullptr);

    if (err_code != NRF_SUCCESS)
    {
        get_logger()->debug("{} Changing of advertisement data failed, {}", role(),
                            testutil::errorToString(err_code));
    }
    else
    {
        m_changeCount++;
        get_logger()->debug("{} Changing advertisement data succeeded (#{})", role(),
                            m_changeCount);
    }

    return err_code;
#else
    return NRF_ERROR_NOT_SUPPORTED;
#endif
}

uint32_t AdapterWrapper::startServiceDiscovery(const uint8_t type, const uint16_t uuid)
{
    uint16_t start_handle = 0x01;
    ble_uuid_t srvc_uuid;

    get_logger()->debug("{} Starting discovery of GATT Primary Services", role());

    srvc_uuid.type = type;
    srvc_uuid.uuid = uuid;

    const auto err_code = sd_ble_gattc_primary_services_discover(
        m_adapter, scratchpad.connection_handle, start_handle, &srvc_uuid);

    if (err_code != NRF_SUCCESS)
    {
        get_logger()->error("{} Failed to initiate or continue a GATT Primary Service Discovery "
                            "procedure, sd_ble_gattc_primary_services_discover {}",
                            role(), testutil::errorToString(err_code));
    }

    return err_code;
}

uint32_t AdapterWrapper::startAuthentication(const bool bond, const bool mitm, const bool lesc,
                                             const bool keypress, const uint8_t ioCaps,
                                             const bool oob, const uint8_t minKeySize,
                                             const uint8_t maxKeySize)
{
    ble_gap_sec_params_t p_sec_params;
    memset(&p_sec_params, 0, sizeof(p_sec_params));
    p_sec_params.bond         = bond ? 1 : 0;
    p_sec_params.mitm         = mitm ? 1 : 0;
    p_sec_params.lesc         = lesc ? 1 : 0;
    p_sec_params.keypress     = keypress ? 1 : 0;
    p_sec_params.io_caps      = ioCaps;
    p_sec_params.oob          = oob ? 1 : 0;
    p_sec_params.min_key_size = minKeySize;
    p_sec_params.max_key_size = maxKeySize;

    const auto err_code =
        sd_ble_gap_authenticate(m_adapter, scratchpad.connection_handle, &p_sec_params);

    if (err_code != NRF_SUCCESS)
    {
        get_logger()->error("{} Error calling sd_ble_gap_authenticate: {}", role(),
                            testutil::errorToString(err_code));
    }

    return err_code;
}

uint32_t AdapterWrapper::authKeyReply(const uint8_t keyType, const uint8_t *key)
{
    const auto err_code =
        sd_ble_gap_auth_key_reply(m_adapter, scratchpad.connection_handle, keyType, key);
    if (err_code != NRF_SUCCESS)
    {
        get_logger()->error("{} Error calling sd_ble_gap_auth_key_reply: {}", role(),
                            testutil::errorToString(err_code));
    }

    return err_code;
}

uint32_t AdapterWrapper::securityParamsReply(const uint8_t status,
                                             const ble_gap_sec_keyset_t &keyset, const bool bond,
                                             const bool mitm, const bool lesc, const bool keypress,
                                             const uint8_t ioCaps, const bool oob,
                                             const uint8_t minKeySize, const uint8_t maxKeySize)
{
    ble_gap_sec_params_t secParams;
    memset(&secParams, 0, sizeof(secParams));
    secParams.bond         = bond ? 1 : 0;
    secParams.mitm         = mitm ? 1 : 0;
    secParams.lesc         = lesc ? 1 : 0;
    secParams.keypress     = keypress ? 1 : 0;
    secParams.io_caps      = ioCaps;
    secParams.oob          = oob ? 1 : 0;
    secParams.min_key_size = minKeySize;
    secParams.max_key_size = maxKeySize;

    const auto err_code = sd_ble_gap_sec_params_reply(m_adapter, scratchpad.connection_handle,
                                                      status, &secParams, &keyset);

    if (err_code != NRF_SUCCESS)
    {
        get_logger()->error("{} sd_ble_gap_sec_params_reply, {}", role(),
                            testutil::errorToString(err_code));
    }

    return err_code;
}

uint32_t AdapterWrapper::securityParamsReply(const ble_gap_sec_keyset_t &keyset)
{
    const auto err_code = sd_ble_gap_sec_params_reply(m_adapter, scratchpad.connection_handle,
                                                      BLE_GAP_SEC_STATUS_SUCCESS, nullptr, &keyset);

    if (err_code != NRF_SUCCESS)
    {
        get_logger()->error("{} sd_ble_gap_sec_params_reply failed, {}", role(),
                            testutil::errorToString(err_code));
    }

    return err_code;
}

uint32_t AdapterWrapper::startCharacteristicDiscovery()
{
    ble_gattc_handle_range_t handle_range;

    handle_range.start_handle = scratchpad.service_start_handle;
    handle_range.end_handle   = scratchpad.service_end_handle;

    get_logger()->debug("{} Discovering characteristics, {}", role(),
                        testutil::asText(handle_range));

    const auto err_code = sd_ble_gattc_characteristics_discover(
        m_adapter, scratchpad.connection_handle, &handle_range);
    if (err_code != NRF_SUCCESS)
    {
        get_logger()->error("{} sd_ble_gattc_characteristics_discover, {}", role(),
                            testutil::errorToString(err_code));
    }

    return err_code;
}

uint32_t AdapterWrapper::startDescriptorDiscovery()
{
    ble_gattc_handle_range_t handle_range;
    get_logger()->debug("{} Discovering characteristic's descriptors", role());

    if (scratchpad.characteristic_decl_handle == 0)
    {
        get_logger()->error("{} No characteristic handle specified.", role());
        return NRF_ERROR_INVALID_STATE;
    }

    handle_range.start_handle = scratchpad.characteristic_decl_handle;
    handle_range.end_handle   = scratchpad.service_end_handle;

    const auto err_code =
        sd_ble_gattc_descriptors_discover(m_adapter, scratchpad.connection_handle, &handle_range);

    if (err_code != NRF_SUCCESS)
    {
        get_logger()->error("{} sd_ble_gattc_descriptors_discover failed, err_code: {}", role(),
                            testutil::errorToString(err_code));
    }

    return err_code;
}

uint32_t AdapterWrapper::writeCCCDValue(const uint16_t cccdHandle, const uint8_t value)
{
    ble_gattc_write_params_t write_params;
    uint8_t cccd_value[2] = {value, 0};

    write_params.handle   = cccdHandle;
    write_params.len      = 2;
    write_params.p_value  = cccd_value;
    write_params.write_op = BLE_GATT_OP_WRITE_REQ;
    write_params.offset   = 0;

    get_logger()->debug("{} Writing to connection {} CCCD handle: {} value: {}", role(),
                        testutil::asText(scratchpad.connection_handle),
                        testutil::asText(cccdHandle), value);

    const auto err_code =
        sd_ble_gattc_write(m_adapter, scratchpad.connection_handle, &write_params);

    if (err_code != NRF_SUCCESS)
    {
        get_logger()->error("{} sd_ble_gattc_write, {}", role(), testutil::errorToString(err_code));
    }

    return err_code;
}

uint32_t AdapterWrapper::writeCharacteristicValue(const uint16_t characteristicHandle,
                                                  const std::vector<uint8_t> &data)
{
    ble_gattc_write_params_t write_params;
    write_params.handle   = characteristicHandle;
    write_params.len      = static_cast<uint16_t>(data.size());
    write_params.p_value  = const_cast<uint8_t *>(data.data());
    write_params.write_op = BLE_GATT_OP_WRITE_REQ;
    write_params.offset   = 0;

    get_logger()->debug(
        "{} Writing to connection_handle: {} characteristic_handle: {} length: {}, value: {:x}",
        role(), testutil::asText(scratchpad.connection_handle),
        testutil::asText(characteristicHandle), data.size(), asHex(data));

    const auto err_code =
        sd_ble_gattc_write(m_adapter, scratchpad.connection_handle, &write_params);
    if (err_code != NRF_SUCCESS)
    {
        get_logger()->error("{} sd_ble_gattc_write, {}", role(), testutil::errorToString(err_code));
    }

    return err_code;
}

adapter_t *AdapterWrapper::unwrap()
{
    return m_adapter;
}

Role AdapterWrapper::role() const
{
    return m_role;
}

std::string AdapterWrapper::port() const
{
    return m_port;
}

void AdapterWrapper::processLog(const sd_rpc_log_severity_t severity,
                                const std::string &log_message)
{
    get_logger()->debug("{}[log] severity:{} message: {}", role(), testutil::asText(severity),
                        log_message);

    if (m_logCallback)
    {
        m_logCallback(severity, log_message);
    }
}

void AdapterWrapper::logEvent(const uint16_t eventId, const ble_gap_evt_t &gapEvent)
{
    switch (eventId)
    {
        case BLE_GAP_EVT_CONNECTED:
            get_logger()->debug("{} BLE_GAP_EVT_CONNECTED [conn_handle:{} connected:[{}]]", role(),
                                testutil::asText(gapEvent.conn_handle),
                                testutil::asText(gapEvent.params.connected));
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            get_logger()->debug("{} BLE_GAP_EVT_DISCONNECTED [conn_handle: {} disconnected:[{}]]",
                                role(), testutil::asText(gapEvent.conn_handle),
                                testutil::asText(gapEvent.params.disconnected));
            break;
        case BLE_GAP_EVT_TIMEOUT:
            get_logger()->debug("{} BLE_GAP_EVT_TIMEOUT [conn_handle: {}  timeout:[{}]]", role(),
                                testutil::asText(gapEvent.conn_handle),
                                testutil::asText(gapEvent.params.timeout));
            break;
        case BLE_GAP_EVT_ADV_REPORT:
            get_logger()->debug("{} BLE_GAP_EVT_ADV_REPORT [conn_handle: {}  adv_report:[{}]]",
                                role(), testutil::asText(gapEvent.conn_handle),
                                testutil::asText(gapEvent.params.adv_report));

            break;
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            get_logger()->debug(
                "{} BLE_GAP_EVT_SEC_PARAMS_REQUEST [conn_handle:{}  sec_params_request:[{}]]",
                role(), testutil::asText(gapEvent.conn_handle),
                testutil::asText(gapEvent.params.sec_params_request));
            break;
        case BLE_GAP_EVT_SEC_INFO_REQUEST:
            get_logger()->debug(
                "{} BLE_GAP_EVT_SEC_INFO_REQUEST [conn_handle:{} sec_info_request:[{}]]", role(),
                testutil::asText(gapEvent.conn_handle),
                testutil::asText(gapEvent.params.sec_info_request));
            break;
        case BLE_GAP_EVT_PASSKEY_DISPLAY:
            get_logger()->debug(
                "{} BLE_GAP_EVT_PASSKEY_DISPLAY [conn_handle:{}  passkey_display:[{}]]", role(),
                testutil::asText(gapEvent.conn_handle),
                testutil::asText(gapEvent.params.passkey_display));
            break;
        case BLE_GAP_EVT_KEY_PRESSED:
            get_logger()->debug("{} BLE_GAP_EVT_KEY_PRESSED [conn_handle:{} key_pressed:[{}]]",
                                role(), testutil::asText(gapEvent.conn_handle),
                                testutil::asText(gapEvent.params.key_pressed));
            break;
        case BLE_GAP_EVT_SEC_REQUEST:
            get_logger()->debug("{} BLE_GAP_EVT_SEC_REQUEST [conn_handle:{} sec_request:[{}]]",
                                role(), testutil::asText(gapEvent.conn_handle),
                                testutil::asText(gapEvent.params.sec_request));
            break;
        case BLE_GAP_EVT_AUTH_KEY_REQUEST:
            get_logger()->debug(
                "{} BLE_GAP_EVT_AUTH_KEY_REQUEST [conn_handle:{} auth_key_request:[{}]]", role(),
                testutil::asText(gapEvent.conn_handle),
                testutil::asText(gapEvent.params.auth_key_request));
            break;
        case BLE_GAP_EVT_LESC_DHKEY_REQUEST:
            get_logger()->debug(
                "{} BLE_GAP_EVT_LESC_DHKEY_REQUEST [conn_handle:{} lesc_dhkey_request:[{}]]",
                role(), testutil::asText(gapEvent.conn_handle),
                testutil::asText(gapEvent.params.lesc_dhkey_request));
            break;
        case BLE_GAP_EVT_CONN_SEC_UPDATE:
            get_logger()->debug(
                "{} BLE_GAP_EVT_CONN_SEC_UPDATE [conn_handle:{} conn_sec_update:[{}]]", role(),
                testutil::asText(gapEvent.conn_handle),
                testutil::asText(gapEvent.params.conn_sec_update));
            break;
        case BLE_GAP_EVT_AUTH_STATUS:
            get_logger()->debug("{} BLE_GAP_EVT_AUTH_STATUS [conn_handle:{} auth_status:[{}]]",
                                role(), testutil::asText(gapEvent.conn_handle),
                                testutil::asText(gapEvent.params.auth_status));

            break;
        case BLE_GAP_EVT_RSSI_CHANGED:
            get_logger()->debug("{} BLE_GAP_EVT_RSSI_CHANGED [conn_handle:{} rssi_changed:[{}]]",
                                role(), testutil::asText(gapEvent.conn_handle),
                                testutil::asText(gapEvent.params.rssi_changed));
            break;
        case BLE_GAP_EVT_SCAN_REQ_REPORT:
            get_logger()->debug(
                "{} BLE_GAP_EVT_SCAN_REQ_REPORT [conn_handle:{} scan_req_report:[{}]]", role(),
                testutil::asText(gapEvent.conn_handle),
                testutil::asText(gapEvent.params.scan_req_report));
            break;
#if NRF_SD_BLE_API == 6
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
            get_logger()->debug(
                "{} BLE_GAP_EVT_PHY_UPDATE_REQUEST [conn_handle:{} scan_req_report:[{}]]", role(),
                testutil::asText(gapEvent.conn_handle),
                testutil::asText(gapEvent.params.phy_update_request));
            break;

        case BLE_GAP_EVT_PHY_UPDATE:
            get_logger()->debug("{} BLE_GAP_EVT_PHY_UPDATE [conn_handle:{} scan_req_report:[{}]]",
                                role(), testutil::asText(gapEvent.conn_handle),
                                testutil::asText(gapEvent.params.phy_update));
            break;

        case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
            get_logger()->debug(
                "{} BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST [conn_handle:{} scan_req_report:[{}]]",
                role(), testutil::asText(gapEvent.conn_handle),
                testutil::asText(gapEvent.params.data_length_update_request));
            break;

        case BLE_GAP_EVT_DATA_LENGTH_UPDATE:
            get_logger()->debug(
                "{} BLE_GAP_EVT_DATA_LENGTH_UPDATE [conn_handle:{} scan_req_report:[{}]]", role(),
                testutil::asText(gapEvent.conn_handle),
                testutil::asText(gapEvent.params.data_length_update));
            break;

        case BLE_GAP_EVT_QOS_CHANNEL_SURVEY_REPORT:
            get_logger()->debug(
                "{} BLE_GAP_EVT_QOS_CHANNEL_SURVEY_REPORT [conn_handle:{}  scan_req_report:[{}]]",
                role(), testutil::asText(gapEvent.conn_handle),
                testutil::asText(gapEvent.params.qos_channel_survey_report));
            break;

        case BLE_GAP_EVT_ADV_SET_TERMINATED:
            get_logger()->debug(
                "{} BLE_GAP_EVT_ADV_SET_TERMINATED [conn_handle:{} scan_req_report:[{}]]", role(),
                testutil::asText(gapEvent.conn_handle),
                testutil::asText(gapEvent.params.adv_set_terminated));
            break;

#endif // NRF_SD_BLE_API == 6
        default:
            get_logger()->debug("{} UNKNOWN EVENT WITH ID [{:x}]", role(), eventId);
            break;
    }
}

void AdapterWrapper::processEvent(const ble_evt_t *p_ble_evt)
{
    auto eventId = p_ble_evt->header.evt_id;

    const auto logGenericUnprocessed = [this, &eventId]() {
        get_logger()->debug("{} Unprocessed event: {:x}", role(), (uint32_t)eventId);
    };

    if (eventId >= BLE_GAP_EVT_BASE && eventId <= BLE_GAP_EVT_LAST)
    {
        logEvent(eventId, p_ble_evt->evt.gap_evt);

        switch (eventId)
        {
            case BLE_GAP_EVT_CONNECTED:
                scratchpad.connection_handle      = p_ble_evt->evt.gap_evt.conn_handle;
                scratchpad.connection_in_progress = false;
                break;
            case BLE_GAP_EVT_DISCONNECTED:
                scratchpad.connection_handle = BLE_CONN_HANDLE_INVALID;
                break;
            case BLE_GAP_EVT_TIMEOUT:
                scratchpad.advertisement_timed_out = true;
                if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
                {
                    scratchpad.connection_in_progress = false;
                }
                break;
            case BLE_GAP_EVT_AUTH_KEY_REQUEST:
                scratchpad.key_type = p_ble_evt->evt.gap_evt.params.auth_key_request.key_type;
                break;
            default:
                break;
        }

        const auto logUnprocessed = [this, &eventId]() {
            get_logger()->debug("{} Unprocessed GAP event, {}", role(),
                                testutil::eventIdAsText(eventId));
        };

        if (m_gapEventCallback)
        {
            if (!m_gapEventCallback(eventId, &(p_ble_evt->evt.gap_evt)))
            {
                logUnprocessed();
            }
        }
        else
        {
            logUnprocessed();
        }
    }
    else if (eventId >= BLE_GATTC_EVT_BASE && eventId <= BLE_GATTC_EVT_LAST)
    {
        const auto logUnprocessed = [this, &eventId]() {
            get_logger()->debug("{}  Unprocessed GATTC event, {}", role(), eventIdAsText(eventId));
        };

        if (m_gattcEventCallback)
        {
            if (!m_gattcEventCallback(eventId, &(p_ble_evt->evt.gattc_evt)))
            {
                logUnprocessed();
            }
        }
        else
        {
            logUnprocessed();
        }
    }
    else if (eventId >= BLE_GATTS_EVT_BASE && eventId <= BLE_GATTS_EVT_LAST)
    {
        const auto logUnprocessed = [this, &eventId]() {
            get_logger()->debug("{} Unprocessed GATTS event, {} ", role(), eventIdAsText(eventId));
        };

        if (m_gattsEventCallback)
        {
            if (!m_gattsEventCallback(eventId, &(p_ble_evt->evt.gatts_evt)))
            {
                logUnprocessed();
            }
        }
        else
        {
            logUnprocessed();
        }
    }
    else
    {
        if (m_eventCallback)
        {
            if (!m_eventCallback(p_ble_evt))
            {
                logGenericUnprocessed();
            }
        }
        else
        {
            logGenericUnprocessed();
        }
    }
}

void AdapterWrapper::processStatus(const sd_rpc_app_status_t code, const std::string &message)
{
    get_logger()->debug("{}[status] code: {} message: {}", role(), testutil::asText(code), message);

    if (m_statusCallback)
    {
        m_statusCallback(code, message);
    }
}

void AdapterWrapper::setStatusCallback(const StatusCallback &statusCallback)
{
    m_statusCallback = statusCallback;
}

void AdapterWrapper::setLogCallback(const LogCallback &logCallback)
{
    m_logCallback = logCallback;
}

void AdapterWrapper::setEventCallback(const EventCallback &eventCallback)
{
    m_eventCallback = eventCallback;
}

void AdapterWrapper::setGattcEventCallback(const GattcEventCallback &callback)
{
    m_gattcEventCallback = callback;
}

void AdapterWrapper::setGattsEventCallback(const GattsEventCallback &callback)
{
    m_gattsEventCallback = callback;
}

void AdapterWrapper::setGapEventCallback(const GapEventCallback &callback)
{
    m_gapEventCallback = callback;
}

void AdapterWrapper::setupScratchpad(const uint16_t mtu)
{
    // Setup scratchpad with default values, take role into account
#if NRF_SD_BLE_API <= 3
    std::memset(&scratchpad.ble_enable_params, 0, sizeof(scratchpad.ble_enable_params));
    scratchpad.ble_enable_params.gatts_enable_params.attr_tab_size =
        BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
    scratchpad.ble_enable_params.gatts_enable_params.service_changed  = false;
    scratchpad.ble_enable_params.gap_enable_params.periph_conn_count  = MaxPeripheralConnections;
    scratchpad.ble_enable_params.gap_enable_params.central_conn_count = MaxCentralConnections;
    scratchpad.ble_enable_params.gap_enable_params.central_sec_count  = MaxCentralSecureConnections;
    scratchpad.ble_enable_params.common_enable_params.p_conn_bw_counts = nullptr;
    scratchpad.ble_enable_params.common_enable_params.vs_uuid_count    = 1;

    if (m_role == Role::Central)
    {
        scratchpad.common_opt.conn_bw.role = BLE_GAP_ROLE_CENTRAL;
    }
    else
    {
        scratchpad.common_opt.conn_bw.role = BLE_GAP_ROLE_PERIPH;
    }

    scratchpad.common_opt.conn_bw.conn_bw.conn_bw_rx = BLE_CONN_BW_HIGH;
    scratchpad.common_opt.conn_bw.conn_bw.conn_bw_tx = BLE_CONN_BW_HIGH;
    scratchpad.opt.common_opt                        = scratchpad.common_opt;
#endif

    scratchpad.mtu = mtu == 0 ? DEFAULT_MTU_SIZE : mtu;

#if NRF_SD_BLE_API == 3
    scratchpad.ble_enable_params.gatt_enable_params.att_mtu = scratchpad.mtu;
#endif

    // Connection parameters
    scratchpad.connection_param.min_conn_interval = millisecondsToUnits(7.5, UNIT_1_25_MS);
    scratchpad.connection_param.max_conn_interval = millisecondsToUnits(7.5, UNIT_1_25_MS);
    scratchpad.connection_param.slave_latency     = 0;
    scratchpad.connection_param.conn_sup_timeout  = millisecondsToUnits(4000, UNIT_10_MS);

    // Scan parameters
    std::memset(&scratchpad.scan_param, 0, sizeof(ble_gap_scan_params_t));
    scratchpad.scan_param.active = 1;

#if NRF_SD_BLE_API == 2
    scratchpad.scan_param.p_whitelist = nullptr;
#endif

#if NRF_SD_BLE_API >= 3 && NRF_SD_BLE_API <= 5
    scratchpad.scan_param.use_whitelist  = 0;
    scratchpad.scan_param.adv_dir_report = 0;
    scratchpad.scan_param.adv_dir_report = 0;
#endif

    scratchpad.scan_param.window   = millisecondsToUnits(50, UNIT_0_625_MS);
    scratchpad.scan_param.timeout  = 0;
    scratchpad.scan_param.interval = millisecondsToUnits(100, UNIT_0_625_MS);

    // Advertisement parameters
#if NRF_SD_BLE_API == 6
    scratchpad.adv_handle = 0; // Using BLE_GAP_ADV_SET_HANDLE_NOT_SET causes a bug
    std::memset(&scratchpad.adv_report_data, 0, sizeof(ble_gap_adv_data_t));
    std::memset(&scratchpad.adv_report_adv_data, 0, sizeof(ble_data_t));
    std::memset(&scratchpad.adv_report_scan_rsp_data, 0, sizeof(ble_data_t));
    std::memset(&scratchpad.adv_properties, 0, sizeof(ble_gap_adv_properties_t));
#endif
    std::memset(&scratchpad.adv_params, 0, sizeof(ble_gap_adv_params_t));

#if NRF_SD_BLE_API <= 5
    scratchpad.adv_params.type    = BLE_GAP_ADV_TYPE_ADV_IND;
    scratchpad.adv_params.fp      = BLE_GAP_ADV_FP_ANY;
    scratchpad.adv_params.timeout = 180; // 180 seconds
#endif
    scratchpad.adv_params.interval    = millisecondsToUnits(40, UNIT_0_625_MS);
    scratchpad.adv_params.p_peer_addr = nullptr; // Undirected advertisement.

#if NRF_SD_BLE_API == 2
    scratchpad.adv_params.p_whitelist = nullptr;
#endif

#if NRF_SD_BLE_API == 6
    scratchpad.adv_properties.type             = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    scratchpad.adv_properties.anonymous        = 0;
    scratchpad.adv_properties.include_tx_power = 0;

    scratchpad.adv_params.properties      = scratchpad.adv_properties;
    scratchpad.adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    scratchpad.adv_params.duration        = BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED;
    scratchpad.adv_params.p_peer_addr     = nullptr;
    scratchpad.adv_params.max_adv_evts    = 0;
    scratchpad.adv_params.primary_phy     = BLE_GAP_PHY_AUTO;
    scratchpad.adv_params.secondary_phy   = BLE_GAP_PHY_AUTO;
    scratchpad.adv_params.channel_mask[0] = 0;
    scratchpad.adv_params.channel_mask[1] = 0;
    scratchpad.adv_params.channel_mask[2] = 0;
    scratchpad.adv_params.channel_mask[3] = 0;
    scratchpad.adv_params.channel_mask[4] = 0;

    scratchpad.adv_report_scan_rsp_data.p_data = nullptr;
    scratchpad.adv_report_scan_rsp_data.len    = 0;

    scratchpad.adv_report_adv_data.p_data = nullptr;
    scratchpad.adv_report_adv_data.len    = 0;
#endif
}

uint32_t AdapterWrapper::setBLEOptions()
{
#if NRF_SD_BLE_API <= 3
    const auto err_code = sd_ble_opt_set(m_adapter, BLE_COMMON_OPT_CONN_BW, &scratchpad.opt);
    if (err_code != NRF_SUCCESS)
    {
        get_logger()->error("{} sd_ble_opt_set, {}", role(), testutil::errorToString(err_code));
    }

    return err_code;
#else
    return NRF_ERROR_NOT_SUPPORTED;
#endif
}

uint32_t AdapterWrapper::initBLEStack()
{
    uint32_t err_code;
    uint32_t *app_ram_base = nullptr;

#if NRF_SD_BLE_API <= 3
    err_code = sd_ble_enable(m_adapter, &scratchpad.ble_enable_params, app_ram_base);
#else
    err_code = sd_ble_enable(m_adapter, app_ram_base);
#endif

    switch (err_code)
    {
        case NRF_SUCCESS:
            break;
        case NRF_ERROR_INVALID_STATE:
            get_logger()->debug("{} BLE stack already enabled", role());
            break;
        default:
            get_logger()->debug("{}  Failed to enable BLE stack, {}", role(),
                                testutil::errorToString(err_code));
            break;
    }

    return err_code;
}

#if NRF_SD_BLE_API >= 5
uint32_t AdapterWrapper::setBLECfg(uint8_t conn_cfg_tag)
{
    const uint32_t ram_start = 0; // Value is not used by ble-driver
    uint32_t error_code;
    ble_cfg_t ble_cfg;

    // Configure the connection roles.
    std::memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count  = MaxPeripheralConnections;
    ble_cfg.gap_cfg.role_count_cfg.central_role_count = MaxCentralConnections;
    ble_cfg.gap_cfg.role_count_cfg.central_sec_count  = MaxCentralSecureConnections;

#if NRF_SD_BLE_API == 6
    ble_cfg.gap_cfg.role_count_cfg.adv_set_count = 1;
#endif

    error_code = sd_ble_cfg_set(m_adapter, BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start);

    if (error_code != NRF_SUCCESS)
    {
        get_logger()->debug(
            "{} sd_ble_cfg_set() failed when attempting to set BLE_GAP_CFG_ROLE_COUNT, {}", role(),
            testutil::errorToString(error_code));
        return error_code;
    }

    std::memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag                 = conn_cfg_tag;
    ble_cfg.conn_cfg.params.gatt_conn_cfg.att_mtu = scratchpad.mtu;

    error_code = sd_ble_cfg_set(m_adapter, BLE_CONN_CFG_GATT, &ble_cfg, ram_start);

    if (error_code != NRF_SUCCESS)
    {
        get_logger()->debug(
            "{} sd_ble_cfg_set() failed when attempting to set BLE_CONN_CFG_GATT, {}", role(),
            testutil::errorToString(error_code));
        return error_code;
    }

    return NRF_SUCCESS;
}
#endif

adapter_t *AdapterWrapper::adapterInit(const char *serial_port, const uint32_t baud_rate,
                                       const uint32_t retransmission_interval,
                                       const uint32_t response_timeout)
{
    const auto phy = sd_rpc_physical_layer_create_uart(
        serial_port, baud_rate, SD_RPC_FLOW_CONTROL_NONE, SD_RPC_PARITY_NONE);

    const auto data_link_layer =
        sd_rpc_data_link_layer_create_bt_three_wire(phy, retransmission_interval);
    const auto transport_layer = sd_rpc_transport_layer_create(data_link_layer, response_timeout);
    return sd_rpc_adapter_create(transport_layer);
}

void AdapterWrapper::statusHandler(adapter_t *adapter, sd_rpc_app_status_t code,
                                   const char *message)
{
    try
    {
        const auto wrappedAdapter = AdapterWrapper::adapters.at(adapter->internal);
        wrappedAdapter->processStatus(code, message);
    }
    catch (std::out_of_range &e)
    {
        get_logger()->error("{:p}, in statusHandler callback, not able to find adapter to invoke "
                            "status handler on, {}",
                            static_cast<void *>(adapter), e.what());
    }
    catch (std::system_error &e)
    {
        get_logger()->error("{:p} std::system_error in statusHandler: {}",
                            static_cast<void *>(adapter), e.what());
    }
}

void AdapterWrapper::eventHandler(adapter_t *adapter, ble_evt_t *p_ble_evt)
{
    try
    {
        const auto wrappedAdapter = AdapterWrapper::adapters.at(adapter->internal);
        wrappedAdapter->processEvent(p_ble_evt);
    }
    catch (std::out_of_range &e)
    {
        get_logger()->error(
            "{:p} in eventHandler, not able to find adapter to invoke event handler on, {}",
            static_cast<void*>(adapter), e.what());
    }
    catch (std::system_error &e)
    {
        get_logger()->error("{:p} in eventHandler, std::system_error, {}",
                            static_cast<void *>(adapter), e.what());
    }
}

void AdapterWrapper::logHandler(adapter_t *adapter, sd_rpc_log_severity_t severity,
                                const char *log_message)
{
    try
    {
        const auto wrappedAdapter = AdapterWrapper::adapters.at(adapter->internal);
        wrappedAdapter->processLog(severity, log_message);
    }
    catch (std::out_of_range &e)
    {
        get_logger()->error(
            "{:p}, in logHandler, not able to find adapter to invoke log handler on, {}",
            static_cast<void *>(adapter), e.what());
    }
    catch (std::system_error &e)
    {
        get_logger()->error("{:p}, in logHandler, std::system_error, {}",
                            static_cast<void *>(adapter), e.what());
    }
}

std::map<void *, AdapterWrapper *> AdapterWrapper::adapters;

} //  namespace testutil
