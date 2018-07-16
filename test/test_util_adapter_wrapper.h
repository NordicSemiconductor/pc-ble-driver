#ifndef TEST_UTIL_ADAPTER_WRAPPER_H__
#define TEST_UTIL_ADAPTER_WRAPPER_H__

#include "test_util_conversion.h"
#include "test_util_role.h"

// Logging support
#include "internal/log.h"

#include "ble.h"
#include "sd_rpc.h"

#include <cstring>

namespace testutil
{
    using LogCallback = std::function<void(const sd_rpc_log_severity_t severity, const std::string &log_message)>;
    using StatusCallback = std::function<void(const sd_rpc_app_status_t code, const std::string &message)>;
    using EventCallback = std::function<bool(const ble_evt_t * p_ble_evt)>;
    using GapEventCallback = std::function<bool(const uint16_t eventId, const ble_gap_evt_t *gapEvent)>;
    using GattsEventCallback = std::function<bool(const uint16_t eventId, const ble_gatts_evt_t *gattsEvent)>;
    using GattcEventCallback = std::function<bool(const uint16_t eventId, const ble_gattc_evt_t *gattcEvent)>;

    // Claculations between milliseconds to SoftDevice API units
    using sd_api_time_unit_t = enum
    {
        UNIT_0_625_MS = 625,  /**< Number of microseconds in 0.625 milliseconds. */
        UNIT_1_25_MS = 1250, /**< Number of microseconds in 1.25 milliseconds. */
        UNIT_10_MS = 10000 /**< Number of microseconds in 10 milliseconds. */
    };

    static uint16_t millisecondsToUnits(const double milliseconds, const sd_api_time_unit_t timeunit)
    {
        const auto microseconds = milliseconds * 1000;
        return (static_cast<uint16_t>(microseconds / timeunit));
    }

    // This struct is used for storing data that the tests can use
    //
    // It will only contain information about one connection, service, characteristic and CCCD
    //
    // NOTICE: 
    //    this struct is not thread safe, the implementer of the test 
    //    must take that into account when using this scratchpad
    struct AdapterWrapperScratchpad
    {
        // Values related to SoftDevice API
        ble_gap_scan_params_t scan_param;
        ble_gap_conn_params_t connection_param;

#if NRF_SD_BLE_API <= 3
        ble_enable_params_t ble_enable_params;
#endif // NRF_SD_BLE_API


#if NRF_SD_BLE_API < 5
        #define DEFAULT_MTU_SIZE  GATT_MTU_SIZE_DEFAULT
#else
        #define DEFAULT_MTU_SIZE BLE_GATT_ATT_MTU_DEFAULT
#endif

#if NRF_SD_BLE_API <= 3
        ble_opt_t        opt;
        ble_common_opt_t common_opt;
#endif

        // Connect handle to connection under test
        uint16_t connection_handle = BLE_CONN_HANDLE_INVALID;

        uint16_t service_start_handle = 0;
        uint16_t service_end_handle = 0;
        
        // Handle to Client Characterstic Configuration Descriptor
        // Can be set during discovery of services
        uint16_t cccd_handle = BLE_GATT_HANDLE_INVALID;
        
        // The service used by the test
        uint16_t service_handle = BLE_GATT_HANDLE_INVALID;
        
        // The characteristic used by the test
        uint16_t characteristic_handle = BLE_GATT_HANDLE_INVALID;

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

        bool send_notifications = false;
        bool advertisement_timed_out = false;

        uint16_t mtu = DEFAULT_MTU_SIZE; // See #define DEFAULT_MTU_SIZE above
        
        // Newer versions of SoftDevice API supports multiple configurations
        uint32_t config_id = 1;

        bool connection_in_progress = false;

        ble_gap_adv_params_t adv_params;
    };

    class AdapterWrapper
    {
    public:
        AdapterWrapper(const Role &role, const std::string &port, const uint32_t baudRate) : m_role(role), m_port(port)
        {
            m_adapter = adapterInit(port.c_str(), baudRate);

            // Setup scratchpad with default values
            setupScratchpad();
        }

        uint32_t configure()
        {
            uint32_t error_code;

#if NRF_SD_BLE_API >= 5
            error_code = setBLECfg(scratchpad.config_id);

            if (error_code != NRF_SUCCESS)
            {
                return error_code;
            }
#endif

            error_code = initBLEStack();

            if (error_code != NRF_SUCCESS)
            {
                return error_code;
            }

#if NRF_SD_BLE_API < 5
            error_code = setBLEOptions();

            if (error_code != NRF_SUCCESS)
            {
                return error_code;
            }
#endif
            
            return error_code;
        }

        uint32_t connect(const ble_gap_addr_t *address)
        {
            const auto err_code = sd_ble_gap_connect(
                m_adapter,
                address,
                &(scratchpad.scan_param),
                &(scratchpad.connection_param)
#if NRF_SD_BLE_API >= 5
                , scratchpad.config_id
#endif
            );

            if (err_code == NRF_SUCCESS)
            {
                scratchpad.connection_in_progress = true;
            }

            return err_code;
        }

        bool error()
        {
            return m_async_error;
        }

        uint32_t startScan()
        {
            uint32_t error_code = sd_ble_gap_scan_start(m_adapter, &scratchpad.scan_param);

            if (error_code != NRF_SUCCESS)
            {
                NRF_LOG(role() << " Scan start failed");
            }
            else
            {
                NRF_LOG(role() << " Scan started");
            }

            return error_code;
        }

        uint32_t startAdvertising()
        {
            if (role() != Peripheral)
            {
                NRF_LOG(role() << " Wrong role, must be peripheral to advertise.");
                return NRF_ERROR_INVALID_STATE;
            }

            uint32_t err_code;

#if NRF_SD_BLE_API <= 3
            err_code = sd_ble_gap_adv_start(m_adapter, &(scratchpad.adv_params));
#else
            err_code = sd_ble_gap_adv_start(m_adapter, &(scratchpad.adv_params), scratchpad.config_id);
#endif

            if (err_code != NRF_SUCCESS)
            {
                NRF_LOG(role() << " Failed to start advertising. " << testutil::errorToString(err_code));
            }

            return err_code;
        }

        uint32_t startServiceDiscovery(const uint8_t type, const uint16_t uuid)
        {
            uint16_t start_handle = 0x01;
            ble_uuid_t srvc_uuid;

            NRF_LOG(role() << " Starting discovery of GATT Primary Services");

            srvc_uuid.type = type;
            srvc_uuid.uuid = uuid;

            const auto err_code = sd_ble_gattc_primary_services_discover(m_adapter,
                scratchpad.connection_handle,
                start_handle,
                &srvc_uuid);

            if (err_code != NRF_SUCCESS)
            {
                NRF_LOG(role() << " Failed to initiate or continue a GATT Primary Service Discovery procedure\n");
            }

            return err_code;
        }

        uint32_t startCharacteristicDiscovery()
        {
            ble_gattc_handle_range_t handle_range;

            handle_range.start_handle = scratchpad.service_start_handle;
            handle_range.end_handle = scratchpad.service_end_handle;

            NRF_LOG(role() << " Discovering characteristics, " << testutil::asText(handle_range));

            return sd_ble_gattc_characteristics_discover(m_adapter, scratchpad.connection_handle, &handle_range);
        }

        uint32_t startDescriptorDiscovery()
        {
            ble_gattc_handle_range_t handle_range;
            NRF_LOG(role() << " Discovering characteristic's descriptors");

            if (scratchpad.characteristic_handle == 0)
            {
                NRF_LOG(role() << " No characteristic handle specified.");
                return NRF_ERROR_INVALID_STATE;
            }

            handle_range.start_handle = scratchpad.characteristic_handle;
            handle_range.end_handle = scratchpad.service_end_handle;

            return sd_ble_gattc_descriptors_discover(m_adapter, scratchpad.connection_handle, &handle_range);
        }

        uint32_t writeCCCDValue(const uint16_t cccdHandle, const uint8_t value)
        {
            ble_gattc_write_params_t write_params;
            uint8_t                  cccd_value[2] = { value, 0 };

            write_params.handle = cccdHandle;
            write_params.len = 2;
            write_params.p_value = cccd_value;
            write_params.write_op = BLE_GATT_OP_WRITE_REQ;
            write_params.offset = 0;

            NRF_LOG(role() << " Writing to connection " 
                << testutil::asText(scratchpad.connection_handle)
                << " CCCD handle: "
                << testutil::asText(cccdHandle)
                << " value: " << value);

            return sd_ble_gattc_write(m_adapter, scratchpad.connection_handle, &write_params);
        }

        uint32_t writeCharacteristicValue(const uint16_t characteristicHandle, const std::vector<uint8_t> &data)
        {
            ble_gattc_write_params_t write_params;
            write_params.handle = characteristicHandle;
            write_params.len = static_cast<uint16_t>(data.size());
            write_params.p_value = const_cast<uint8_t*>(data.data());
            write_params.write_op = BLE_GATT_OP_WRITE_REQ;
            write_params.offset = 0;

            NRF_LOG(role() << " Writing to connection_handle: "
                << testutil::asText(scratchpad.connection_handle)
                << " characteristic_handle: "
                << testutil::asText(characteristicHandle)
                << " length: " << data.size()
                << " value: " << asHex(data));

            return sd_ble_gattc_write(m_adapter, scratchpad.connection_handle, &write_params);
        }

        adapter_t* unwrap()
        {
            return m_adapter;
        }

        Role role() const
        {
            return m_role;
        }

        std::string port() const
        {
            return m_port;
        }

        void processLog(const sd_rpc_log_severity_t severity, const std::string &log_message)
        {
            NRF_LOG(role() << "[log] severity:" << testutil::asText(severity) << " message:" << log_message);

            if (m_logCallback != nullptr)
            {
                m_logCallback(severity, log_message);
            }
        }

        void processEvent(const ble_evt_t * p_ble_evt)
        {
            auto eventId = p_ble_evt->header.evt_id;

            if (eventId >= BLE_GAP_EVT_BASE && eventId <= BLE_GAP_EVT_LAST)
            {
                if (m_gapEventCallback != nullptr)
                {
                    if (!m_gapEventCallback(eventId, &(p_ble_evt->evt.gap_evt)))
                    {
                        NRF_LOG(role() << " Unprocessed GAP event: 0x" << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)eventId);
                    }
                }
            }
            else if (eventId >= BLE_GATTC_EVT_BASE && eventId <= BLE_GATTC_EVT_LAST)
            {
                if (m_gattcEventCallback != nullptr)
                {
                    if (!m_gattcEventCallback(eventId, &(p_ble_evt->evt.gattc_evt)))
                    {
                        NRF_LOG(role() <<  " Unprocessed GATTC event: 0x" << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)eventId);
                    }
                }
            }
            else if (eventId >= BLE_GATTS_EVT_BASE && eventId <= BLE_GATTS_EVT_LAST)
            {
                if (m_gattsEventCallback != nullptr)
                {
                    if (!m_gattsEventCallback(eventId, &(p_ble_evt->evt.gatts_evt)))
                    {
                        NRF_LOG(role() << " Unprocessed GATTS event: 0x" << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)eventId);
                    }
                }
            }
            else if (m_eventCallback != nullptr)
            {
                if (!m_eventCallback(p_ble_evt))
                {
                    NRF_LOG(role() << " Unprocessed event: 0x" << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)eventId);
                }
            }
        }

        void processStatus(const sd_rpc_app_status_t code, const std::string &message)
        {
            NRF_LOG(role() << "[status] code:" << testutil::asText(code) << " message:" << message);

            if (m_statusCallback != nullptr)
            {
                m_statusCallback(code, message);
            }
        }

        void setStatusCallback(const StatusCallback &statusCallback)
        {
            m_statusCallback = statusCallback;
        }

        void setLogCallback(const LogCallback &logCallback)
        {
            m_logCallback = logCallback;
        }

        void setEventCallback(const EventCallback &eventCallback)
        {
            m_eventCallback = eventCallback;
        }

        void setGattcEventCallback(const GattcEventCallback &callback)
        {
            m_gattcEventCallback = callback;
        }

        void setGattsEventCallback(const GattsEventCallback &callback)
        {
            m_gattsEventCallback = callback;
        }

        void setGapEventCallback(const GapEventCallback &callback)
        {
            m_gapEventCallback = callback;
        }

        // Public data members used during testing
    public:
        // Scratchpad for values related to SoftDevice API.
        
        // The scratchpad is used by methods in this class and by implementers of test.
        
        // There is no encapsulation of the values in the scratchpad and the scratchpad is not
        // thread safe.
        AdapterWrapperScratchpad scratchpad;

    private:
        // Adapter from pc-ble-driver
        adapter_t *m_adapter;
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

        // The role given in the test. The SoftDevice supports multi-role, but for 
        // simplicity in the tests we try to keep then to one role.
        Role m_role;

        void setupScratchpad()
        {
            // Setup scratchpad with default values
#if NRF_SD_BLE_API <= 3
            std::memset(&scratchpad.ble_enable_params, 0, sizeof(scratchpad.ble_enable_params));
#endif

#if NRF_SD_BLE_API == 3
            scratchpad.ble_enable_params.gatt_enable_params.att_mtu = GATT_MTU_SIZE_DEFAULT;
#elif NRF_SD_BLE_API < 3
            scratchpad.ble_enable_params.gatts_enable_params.attr_tab_size = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
            scratchpad.ble_enable_params.gatts_enable_params.service_changed = false;
            scratchpad.ble_enable_params.gap_enable_params.periph_conn_count = 1;
            scratchpad.ble_enable_params.gap_enable_params.central_conn_count = 1;
            scratchpad.ble_enable_params.gap_enable_params.central_sec_count = 1;
            scratchpad.ble_enable_params.common_enable_params.p_conn_bw_counts = NULL;
            scratchpad.ble_enable_params.common_enable_params.vs_uuid_count = 1;
#endif

#if NRF_SD_BLE_API <= 3
            scratchpad.common_opt.conn_bw.role = BLE_GAP_ROLE_CENTRAL;
            scratchpad.common_opt.conn_bw.conn_bw.conn_bw_rx = BLE_CONN_BW_HIGH;
            scratchpad.common_opt.conn_bw.conn_bw.conn_bw_tx = BLE_CONN_BW_HIGH;
            scratchpad.opt.common_opt = scratchpad.common_opt;
#endif

            // Connection parameters
            scratchpad.connection_param.min_conn_interval = millisecondsToUnits(7.5, UNIT_1_25_MS);
            scratchpad.connection_param.max_conn_interval = millisecondsToUnits(7.5, UNIT_1_25_MS);
            scratchpad.connection_param.slave_latency = 0;
            scratchpad.connection_param.conn_sup_timeout = millisecondsToUnits(4000, UNIT_10_MS);

            // Scan parameters
            memset(&scratchpad.scan_param, 0, sizeof(ble_gap_scan_params_t));
            scratchpad.scan_param.active = 1;
#if NRF_SD_BLE_API == 2
            scratchpad.scan_param.p_whitelist = nullptr;
#endif
#if NRF_SD_BLE_API >= 3
            scratchpad.scan_param.use_whitelist = 0;
            scratchpad.scan_param.adv_dir_report = 0;
            scratchpad.scan_param.adv_dir_report = 0;
#endif

            scratchpad.scan_param.window = millisecondsToUnits(50, UNIT_0_625_MS);
            scratchpad.scan_param.timeout = 0;
            scratchpad.scan_param.interval = millisecondsToUnits(100, UNIT_0_625_MS);

            // Advertisement parameters
            memset(&scratchpad.adv_params, 0, sizeof(ble_gap_adv_params_t));
            scratchpad.adv_params.type = BLE_GAP_ADV_TYPE_ADV_IND;
            scratchpad.adv_params.p_peer_addr = NULL;                        // Undirected advertisement.
            scratchpad.adv_params.fp = BLE_GAP_ADV_FP_ANY;
            scratchpad.adv_params.interval = millisecondsToUnits(40, UNIT_0_625_MS);
            scratchpad.adv_params.timeout = 180; // 180 seconds

#if NRF_SD_BLE_API == 2
            scratchpad.adv_params.p_whitelist = NULL;
#endif
        }

        uint32_t setBLEOptions()
        {
#if NRF_SD_BLE_API <= 3
            return sd_ble_opt_set(m_adapter, BLE_COMMON_OPT_CONN_BW, &scratchpad.opt);
#else
            return NRF_ERROR_NOT_SUPPORTED;
#endif
        }

        uint32_t initBLEStack()
        {
            uint32_t            err_code;
            uint32_t *          app_ram_base = NULL;

#if NRF_SD_BLE_API <= 3
            err_code = sd_ble_enable(m_adapter, &scratchpad.ble_enable_params, app_ram_base);
#else
            err_code = sd_ble_enable(m_adapter, app_ram_base);
#endif

            switch (err_code) {
            case NRF_SUCCESS:
                break;
            case NRF_ERROR_INVALID_STATE:
                NRF_LOG(role() << " BLE stack already enabled\n");
                break;
            default:
                NRF_LOG(role() << " Failed to enable BLE stack. " << testutil::errorToString(err_code));
                break;
            }

            return err_code;
        }

#if NRF_SD_BLE_API >= 5
        uint32_t setBLECfg(uint8_t conn_cfg_tag)
        {
            const uint32_t ram_start = 0; // Value is not used by ble-driver
            uint32_t error_code;
            ble_cfg_t ble_cfg;

            // Configure the connection roles.
            memset(&ble_cfg, 0, sizeof(ble_cfg));
            ble_cfg.gap_cfg.role_count_cfg.periph_role_count = 1;
            ble_cfg.gap_cfg.role_count_cfg.central_role_count = 1;
            ble_cfg.gap_cfg.role_count_cfg.central_sec_count = 1;

            error_code = sd_ble_cfg_set(m_adapter, BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start);

            if (error_code != NRF_SUCCESS)
            {
                NRF_LOG(role() << " sd_ble_cfg_set() failed when attempting to set BLE_GAP_CFG_ROLE_COUNT. " << testutil::errorToString(error_code));
                return error_code;
            }

            memset(&ble_cfg, 0x00, sizeof(ble_cfg));
            ble_cfg.conn_cfg.conn_cfg_tag = conn_cfg_tag;
            ble_cfg.conn_cfg.params.gatt_conn_cfg.att_mtu = scratchpad.mtu;

            error_code = sd_ble_cfg_set(m_adapter, BLE_CONN_CFG_GATT, &ble_cfg, ram_start);
            if (error_code != NRF_SUCCESS)
            {
                NRF_LOG(role() << " sd_ble_cfg_set() failed when attempting to set BLE_CONN_CFG_GATT. " << testutil::errorToString(error_code));
                return error_code;
            }

            return NRF_SUCCESS;
        }
#endif

        adapter_t *adapterInit(const char * serial_port, uint32_t baud_rate)
        {
            physical_layer_t  * phy;
            data_link_layer_t * data_link_layer;
            transport_layer_t * transport_layer;

            phy = sd_rpc_physical_layer_create_uart(serial_port, baud_rate, SD_RPC_FLOW_CONTROL_NONE,
                SD_RPC_PARITY_NONE);
            data_link_layer = sd_rpc_data_link_layer_create_bt_three_wire(phy, 100);
            transport_layer = sd_rpc_transport_layer_create(data_link_layer, 100);
            return sd_rpc_adapter_create(transport_layer);
        }
    };

    std::ostream& operator<< (std::ostream &s, const ble_gap_addr_t *address)
    {
        const int address_length = 6;

        for (int i = sizeof(address->addr) - 1; i >= 0; --i)
        {
            s << std::setfill('0') << std::setw(2) << std::hex << static_cast<unsigned int>(address->addr[i]);
            
            if (i < 5)
            {
                s << ":";
            }
        }

        s << "/";

        switch (address->addr_type)
        {
        case BLE_GAP_ADDR_TYPE_PUBLIC:
            s << "PUBLIC";
            break;
        case BLE_GAP_ADDR_TYPE_RANDOM_STATIC:
            s << "RANDOM_STATIC";
            break;
        case BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE:
            s << "RANDOM_PRIVATE_RESOLVABLE";
            break;
        case BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE:
            s << "RANDOM_PRIVATE_NON_RESOLVABLE";
            break;
        default:
            s << "UNKNOWN";
            break;
        }

        return s;
    }
} //  namespace testutil

#endif // TEST_UTIL_ADAPTER_WRAPPER_H__
