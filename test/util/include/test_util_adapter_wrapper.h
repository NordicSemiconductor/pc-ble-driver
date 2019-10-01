#pragma once

#if NRF_SD_BLE_API == 4 || NRF_SD_BLE_API > 6
#error "wrapper does not take into account this version of the SoftDevice API."
#endif

#include "test_util_conversion.h"
#include "test_util_role.h"

#include "ble.h"
#include "sd_rpc.h"

#include "test_util_adapter_wrapper_scratchpad.h"

#include <array>
#include <cstring>
#include <functional>
#include <map>
#include <vector>

namespace testutil {
using LogCallback =
    std::function<void(const sd_rpc_log_severity_t severity, const std::string &log_message)>;
using StatusCallback =
    std::function<void(const sd_rpc_app_status_t code, const std::string &message)>;
using EventCallback    = std::function<bool(const ble_evt_t *p_ble_evt)>;
using GapEventCallback = std::function<bool(const uint16_t eventId, const ble_gap_evt_t *gapEvent)>;
using GattsEventCallback =
    std::function<bool(const uint16_t eventId, const ble_gatts_evt_t *gattsEvent)>;
using GattcEventCallback =
    std::function<bool(const uint16_t eventId, const ble_gattc_evt_t *gattcEvent)>;

// Calculations between milliseconds to SoftDevice API units
using sd_api_time_unit_t = enum {
    UNIT_0_625_MS = 625,  /**< Number of microseconds in 0.625 milliseconds. */
    UNIT_1_25_MS  = 1250, /**< Number of microseconds in 1.25 milliseconds. */
    UNIT_10_MS    = 10000 /**< Number of microseconds in 10 milliseconds. */
};

uint16_t millisecondsToUnits(const double milliseconds, const sd_api_time_unit_t timeunit);

constexpr uint8_t DATA_ROTATING_BUFFER_COUNT = 4;

using data_buffer  = std::vector<uint8_t>;
using data_buffers = std::vector<data_buffer>;

class AdapterWrapper
{
  public:
    AdapterWrapper(const Role &role, const std::string &port, const uint32_t baudRate,
                   const uint16_t mtu = 0, const uint32_t retransmissionInterval = 250,
                   uint32_t const responseTimeout = 1000);
    ~AdapterWrapper();

    // Static data member that keeps pointer
    // to all adapters used by the tests
    static std::map<void *, AdapterWrapper *> adapters;

    uint32_t configure();

    uint32_t connect(const ble_gap_addr_t *address);
    bool error() const;

    uint32_t open();

    uint32_t close();

    uint32_t startScan(const bool resume = false, const bool extended = false,
                       const bool incomplete = false);

    uint32_t setupAdvertising(const std::vector<uint8_t> &advertisingData  = std::vector<uint8_t>{},
                              const std::vector<uint8_t> &scanResponseData = std::vector<uint8_t>{},
                              const uint32_t interval = BLE_GAP_ADV_INTERVAL_MIN,
                              const uint32_t duration = 0, const bool connectable = true,
                              const bool extended = false, const bool scan_req_notification = false,
                              const uint8_t set_id = 0, const uint8_t primary_phy = 0,
                              const uint8_t secondary_phy = 0, const uint8_t filter_policy = 0,
                              const uint32_t max_adv_events = 0);

    uint32_t startAdvertising();

    uint32_t changeAdvertisingData(const std::vector<uint8_t> &advertisingData,
                                   const std::vector<uint8_t> &scanResponseData);

    uint32_t startServiceDiscovery(const uint8_t type, const uint16_t uuid);

    uint32_t startAuthentication(const bool bond = true, const bool mitm = true,
                                 const bool lesc = false, const bool keypress = false,
                                 const uint8_t ioCaps = BLE_GAP_IO_CAPS_KEYBOARD_DISPLAY,
                                 const bool oob = false, const uint8_t minKeySize = 7,
                                 const uint8_t maxKeySize = 16);

    uint32_t authKeyReply(const uint8_t keyType, const uint8_t *key);

    uint32_t securityParamsReply(const uint8_t status, const ble_gap_sec_keyset_t &keyset,
                                 const bool bond = true, const bool mitm = true,
                                 const bool lesc = false, const bool keypress = false,
                                 const uint8_t ioCaps = BLE_GAP_IO_CAPS_KEYBOARD_DISPLAY,
                                 const bool oob = false, const uint8_t minKeySize = 7,
                                 const uint8_t maxKeySize = 16);

    uint32_t securityParamsReply(const ble_gap_sec_keyset_t &keyset);

    uint32_t startCharacteristicDiscovery();

    uint32_t startDescriptorDiscovery();

    uint32_t writeCCCDValue(const uint16_t cccdHandle, const uint8_t value);

    uint32_t writeCharacteristicValue(const uint16_t characteristicHandle,
                                      const std::vector<uint8_t> &data);

    adapter_t *unwrap();

    Role role() const;
    std::string port() const;

    void processLog(const sd_rpc_log_severity_t severity, const std::string &log_message);
    void logEvent(const uint16_t eventId, const ble_gap_evt_t &gapEvent);

    void processEvent(const ble_evt_t *p_ble_evt);
    void processStatus(const sd_rpc_app_status_t code, const std::string &message);

    void setStatusCallback(const StatusCallback &statusCallback);
    void setLogCallback(const LogCallback &logCallback);
    void setEventCallback(const EventCallback &eventCallback);
    void setGattcEventCallback(const GattcEventCallback &callback);
    void setGattsEventCallback(const GattsEventCallback &callback);
    void setGapEventCallback(const GapEventCallback &callback);

    // Public data members used during testing
  public:
    // Scratchpad for values related to SoftDevice API.

    // The scratchpad is used by methods in this class and by implementers of test.

    // There is no encapsulation of the values in the scratchpad and the scratchpad is not
    // thread safe.
    AdapterWrapperScratchpad scratchpad;

    // My address
    ble_gap_addr_t address;

  private:
    // Adapter from pc-ble-driver
    adapter_t *m_adapter;

    // The role given in the test. The SoftDevice supports multi-role, but for
    // simplicity in the tests we try to keep then to one role.
    const Role m_role;

    const std::string m_port;

    // Used to indicate that something went wrong in an async callback
    // Since the test framework is not thread safe, this variable is used
    // indicate error in callbacks
    bool m_async_error;

    // Callbacks
    LogCallback m_logCallback;
    StatusCallback m_statusCallback;
    EventCallback m_eventCallback;
    GapEventCallback m_gapEventCallback;
    GattsEventCallback m_gattsEventCallback;
    GattcEventCallback m_gattcEventCallback;

    void setupScratchpad(const uint16_t mtu = 0);

    uint32_t setBLEOptions();

    uint32_t initBLEStack();

#if NRF_SD_BLE_API >= 5
    uint32_t setBLECfg(uint8_t conn_cfg_tag);
#endif

    // This data member is a way to change the advertising data while advertising is in progress.
    // The way SoftDevice V6 works is that the pointer address needs to be different when changing
    // the advertising data while advertising.
#if NRF_SD_BLE_API > 5
    data_buffers m_data_buffers{};
    uint8_t m_data_buffers_index{0};
    data_buffer &nextDataBuffer();
    uint32_t m_changeCount{0};
#endif

    adapter_t *adapterInit(const char *serial_port, const uint32_t baud_rate,
                           const uint32_t retransmission_interval, const uint32_t response_timeout);

    static void statusHandler(adapter_t *adapter, sd_rpc_app_status_t code, const char *message);

    static void eventHandler(adapter_t *adapter, ble_evt_t *p_ble_evt);

    static void logHandler(adapter_t *adapter, sd_rpc_log_severity_t severity,
                           const char *log_message);
};

} //  namespace testutil
