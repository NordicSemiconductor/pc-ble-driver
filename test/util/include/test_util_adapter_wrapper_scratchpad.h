#pragma once

#include "test_util_conversion.h"
#include "test_util_role.h"

#include "ble.h"

constexpr size_t SCAN_RSP_DATA_BUFFER_SIZE = 1000;

#if NRF_SD_BLE_API == 4 || NRF_SD_BLE_API > 6
#error "wrapper does not take into account this version of the SoftDevice API."
#endif

namespace testutil {

// This struct is used for storing data that the tests can use
//
// It will only contain information about one connection, service, characteristic and CCCD
//
// NOTICE:
//    this struct is not thread safe, the implementer of the test
//    must take that into account when using this scratchpad

#if NRF_SD_BLE_API < 6
constexpr size_t ADV_DATA_BUFFER_SIZE = BLE_GAP_ADV_MAX_SIZE;
#else
constexpr size_t ADV_DATA_BUFFER_SIZE =
    BLE_GAP_ADV_SET_DATA_SIZE_EXTENDED_MAX_SUPPORTED;
#endif

struct AdapterWrapperScratchpad
{
    // Values related to SoftDevice API
    ble_gap_scan_params_t scan_param;
    ble_gap_conn_params_t connection_param;

#if NRF_SD_BLE_API <= 3
    ble_enable_params_t ble_enable_params;
#endif // NRF_SD_BLE_API

#if NRF_SD_BLE_API < 5
#define DEFAULT_MTU_SIZE GATT_MTU_SIZE_DEFAULT
#else
#define DEFAULT_MTU_SIZE BLE_GATT_ATT_MTU_DEFAULT
#endif

#if NRF_SD_BLE_API <= 3
    ble_opt_t opt;
    ble_common_opt_t common_opt;
#endif

    // Connect handle to connection under test
    uint16_t connection_handle = BLE_CONN_HANDLE_INVALID;

    // Security related values
    uint8_t key_type;
    uint8_t key[16];

    uint16_t service_start_handle = 0;
    uint16_t service_end_handle   = 0;

    // Handle to Client Characterstic Configuration Descriptor
    // Can be set during discovery of services
    uint16_t cccd_handle = BLE_GATT_HANDLE_INVALID;

    // The service used by the test
    uint16_t service_handle = BLE_GATT_HANDLE_INVALID;

    // The characteristic declaration handle used by the test
    uint16_t characteristic_decl_handle = BLE_GATT_HANDLE_INVALID;

    // The characteristic value handle used by the test
    uint16_t characteristic_value_handle = BLE_GATT_HANDLE_INVALID;

    // The descriptor used by the test
    uint16_t descriptor_handle = BLE_GATT_HANDLE_INVALID;

    // Target service UUID to use during testing
    ble_uuid_t target_service;

    // Target characteristic UUID to use during testing
    ble_uuid_t target_characteristic;

    // Target descriptor UUID to use during testing
    ble_uuid_t target_descriptor;

    // The GATT server side representation of the characteristic used by the test
    ble_gatts_char_handles_t gatts_characteristic_handle;

    bool send_notifications      = false;
    bool advertisement_timed_out = false;

    uint16_t mtu = DEFAULT_MTU_SIZE; // See #define DEFAULT_MTU_SIZE above

    // Newer versions of SoftDevice API supports multiple configurations
    uint32_t config_id = 1;

    bool connection_in_progress = false;

#if NRF_SD_BLE_API == 6
    // Data members related to receiving advertisement reports
    ble_data_t adv_report_receive_buffer{};
    uint8_t adv_report_data_received[ADV_DATA_BUFFER_SIZE]; // Data to store advertisement report in

    // Data members related to sending advertisement reports
    uint8_t adv_handle{BLE_GAP_ADV_SET_HANDLE_NOT_SET};
    ble_gap_adv_data_t adv_report_data{};                       // Data used for advertising
    ble_data_t adv_report_adv_data{};                           // Advertisement data
    uint8_t adv_report_adv_data_buffer[ADV_DATA_BUFFER_SIZE]{}; // Advertisement data buffer
    uint8_t adv_report_scan_rsp_data_buffer[SCAN_RSP_DATA_BUFFER_SIZE]{};            // Advertisement data buffer
    ble_data_t adv_report_scan_rsp_data{};                      // Scan report data
    ble_gap_adv_properties_t adv_properties{};                  // Properties used for advertising
#endif

    ble_gap_adv_params_t adv_params{}; // Parameters used for advertising
};
} //  namespace testutil
