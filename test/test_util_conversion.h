#ifndef TEST_UTIL_CONVERSION_H__
#define TEST_UTIL_CONVERSION_H__

#include "ble.h"

namespace testutil
{
    /**
    * @brief Function that convert data to string representation
    * @param[in] data Data to represent as a string
    * @return string representation of data
    */
    static std::string asHex(const std::vector<uint8_t> &data) {
        std::stringstream retval;

        for (uint8_t const& value : data)
        {
            retval << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(value) << ' ';
        }

        return retval.str();
    }

    static std::string asHex(const uint16_t &data)
    {
        std::stringstream retval;
        retval << std::setfill('0') << std::setw(2) << std::hex << data;
        return retval.str();
    }

    /**
    * @brief Function that convert a Bluetooth Low Energy address to string
    *
    * @param[in] address Bluetooth Lower Energy address
    * @return string representation of provided address
    */
    static std::string asText(const ble_gap_addr_t &address)
    {
        std::stringstream retval;

        for (int i = sizeof(address.addr) - 1; i >= 0; --i)
        {
            retval << std::setfill('0') << std::setw(2) << std::hex << static_cast<unsigned int>(address.addr[i]);

            if (i > 0)
            {
                retval << ":";
            }
        }

        retval << "/";

        switch (address.addr_type)
        {
        case BLE_GAP_ADDR_TYPE_PUBLIC:
            retval << "PUBLIC";
            break;
        case BLE_GAP_ADDR_TYPE_RANDOM_STATIC:
            retval << "RANDOM_STATIC";
            break;
        case BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE:
            retval << "RANDOM_PRIVATE_RESOLVABLE";
            break;
        case BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE:
            retval << "RANDOM_PRIVATE_NON_RESOLVABLE";
            break;
        default:
            retval << "UNKNOWN";
            break;
        }

        return retval.str();
    }

    /**
    * @brief Function that convert an error code to string representation
    *
    * param[in] error code
    *
    * @return textual representation of the error
    */
    static std::string errorToString(const uint32_t error_code)
    {
        std::stringstream retval;
        retval << "error code: 0x" << std::setfill('0') << std::setw(2) << std::hex << error_code;
        return  retval.str();
    }

    /**
    * @brief Function that convert a GATT status code to string representation
    *
    * param[in] code code
    *
    * @return textual representation of the code
    */
    static std::string gattStatusToString(const uint16_t code)
    {
        std::stringstream retval;
        retval << "GATT status code: 0x" << std::setfill('0') << std::setw(4) << std::hex << (uint32_t)code << ".";
        return  retval.str();
    }

    // Operator overloading for common types used
    std::string asText(const sd_rpc_app_status_t &status)
    {
        switch (status)
        {
        case PKT_SEND_MAX_RETRIES_REACHED:
            return "PKT_SEND_MAX_RETRIES_REACHED";
        case PKT_UNEXPECTED:
            return "PKT_UNEXPECTED";
        case PKT_ENCODE_ERROR:
            return "PKT_ENCODE_ERROR";
        case PKT_DECODE_ERROR:
            return "PKT_DECODE_ERROR";
        case PKT_SEND_ERROR:
            return "PKT_SEND_ERROR";
        case IO_RESOURCES_UNAVAILABLE:
            return "IO_RESOURCES_UNAVAILABLE";
        case RESET_PERFORMED:
            return "RESET_PERFORMED";
        case CONNECTION_ACTIVE:
            return "CONNECTION_ACTIVE";
        default:
            return "UNKNOWN";
        }
    }

    static std::string asText(const uint16_t &value)
    {
        std::stringstream retval;
        retval << "0x" << std::setfill('0') << std::setw(4) << std::hex << (uint32_t)value;
        return retval.str();
    }

    static std::string asText(const uint8_t &value)
    {
        std::stringstream retval;
        retval << "0x" << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)value;
        return retval.str();
    }

    static std::string asText(const ble_uuid_t &uuid)
    {
        std::stringstream retval;
        retval << "uuid: [type: " << asText(uuid.type) << ", uuid: " << asText(uuid.uuid) << "]";
        return retval.str();
    }

    static std::string asText(const ble_gattc_char_t &gattc_char)
    {
        std::stringstream retval;
        retval << "characteristic: ["
            << "handle_decl: " << asText(gattc_char.handle_decl)
            << ", handle_value: " << asText(gattc_char.handle_value)
            << ", uuid: " << asText(gattc_char.uuid)
            << "]";

        return retval.str();
    }

    static std::string asText(const ble_gattc_handle_range_t &gattc_handle_range)
    {
        std::stringstream retval;
        retval << "handle range: ["
            << "start: " << asText(gattc_handle_range.start_handle)
            << ", end: " << asText(gattc_handle_range.end_handle)
            << "]";
        return retval.str();
    }

    static std::string asText(const ble_gattc_desc_t &gattc_desc)
    {
        std::stringstream retval;
        retval << "descriptor: ["
            << "handle: " << asText(gattc_desc.handle)
            << " " << asText(gattc_desc.uuid)
            << "]";
        return retval.str();
    }

    static std::string asText(const sd_rpc_log_severity_t &severity)
    {
        switch (severity)
        {
        case SD_RPC_LOG_TRACE:
            return "TRACE";
            break;
        case SD_RPC_LOG_DEBUG:
            return "DEBUG";
            break;
        case SD_RPC_LOG_INFO:
            return "INFO";
            break;
        case SD_RPC_LOG_WARNING:
            return "WARNING";
            break;
        case SD_RPC_LOG_ERROR:
            return "ERROR";
            break;
        case SD_RPC_LOG_FATAL:
            return "FATAL";
            break;
        default:
            return "UNKNOWN";
            break;
        };
    }
} // namespace testutil

#endif // TEST_UTIL_CONVERSION_H__
