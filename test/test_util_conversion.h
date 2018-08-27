#ifndef TEST_UTIL_CONVERSION_H__
#define TEST_UTIL_CONVERSION_H__

#include "ble.h"

namespace testutil {
/**
 * @brief Function that convert data to string representation
 * @param[in] data Data to represent as a string
 * @return string representation of data
 */
static std::string asHex(const std::vector<uint8_t> &data) {
    std::stringstream retval;

    for (uint8_t const &value : data)
    {
        retval << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(value) << ' ';
    }

    return retval.str();
}

static std::string asHex(const uint16_t &data) {
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
static std::string asText(const ble_gap_addr_t &address) {
    const int address_length = 6;
    std::stringstream retval;

    for (int i = sizeof(address.addr) - 1; i >= 0; --i)
    {
        retval << std::setfill('0') << std::setw(2) << std::hex
               << static_cast<unsigned int>(address.addr[i]);

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
static std::string errorToString(const uint32_t error_code) {
    std::stringstream retval;
    retval << "error code: 0x" << std::setfill('0') << std::setw(2) << std::hex << error_code;
    return retval.str();
}

static std::string ioCapsToString(const uint8_t code) {
    std::stringstream retval;

    switch (code)
    {
        case BLE_GAP_IO_CAPS_DISPLAY_ONLY:
            retval << " BLE_GAP_IO_CAPS_DISPLAY_ONLY";
            break;
        case BLE_GAP_IO_CAPS_DISPLAY_YESNO:
            retval << " BLE_GAP_IO_CAPS_DISPLAY_YESNO";
            break;
        case BLE_GAP_IO_CAPS_KEYBOARD_ONLY:
            retval << " BLE_GAP_IO_CAPS_KEYBOARD_ONLY";
            break;
        case BLE_GAP_IO_CAPS_NONE:
            retval << " BLE_GAP_IO_CAPS_NONE";
            break;
        case BLE_GAP_IO_CAPS_KEYBOARD_DISPLAY:
            retval << " BLE_GAP_IO_CAPS_KEYBOARD_DISPLAY";
            break;
        default:
            retval << " UNKNOWN";
            break;
    }

    retval << "/0x" << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)code << ".";
    return retval.str();
}

static std::string gattAuthErrorSrcToString(const uint8_t errorSrc) {
    std::stringstream retval;

    switch (errorSrc)
    {
        case BLE_GAP_SEC_STATUS_SOURCE_LOCAL:
            retval << "BLE_GAP_SEC_STATUS_SOURCE_LOCAL";
            break;
        case BLE_GAP_SEC_STATUS_SOURCE_REMOTE:
            retval << "BLE_GAP_SEC_STATUS_SOURCE_REMOTE";
            break;
        default:
            retval << "unknown";
            break;
    }

    retval << "/0x" << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)errorSrc << ".";
    return retval.str();
}

static std::string gattAuthStatusToString(const uint8_t authStatus) {
    std::stringstream retval;

    switch (authStatus)
    {
        case BLE_GAP_SEC_STATUS_SUCCESS:
            retval << "BLE_GAP_SEC_STATUS_SUCCESS";
            break;
        case BLE_GAP_SEC_STATUS_TIMEOUT:
            retval << "BLE_GAP_SEC_STATUS_TIMEOUT";
            break;
        case BLE_GAP_SEC_STATUS_PDU_INVALID:
            retval << "BLE_GAP_SEC_STATUS_PDU_INVALID";
            break;
        case BLE_GAP_SEC_STATUS_RFU_RANGE1_BEGIN:
            retval << "BLE_GAP_SEC_STATUS_RFU_RANGE1_BEGIN";
            break;
        case BLE_GAP_SEC_STATUS_RFU_RANGE1_END:
            retval << "BLE_GAP_SEC_STATUS_RFU_RANGE1_END";
            break;
        case BLE_GAP_SEC_STATUS_PASSKEY_ENTRY_FAILED:
            retval << "BLE_GAP_SEC_STATUS_PASSKEY_ENTRY_FAILED";
            break;
        case BLE_GAP_SEC_STATUS_OOB_NOT_AVAILABLE:
            retval << "BLE_GAP_SEC_STATUS_OOB_NOT_AVAILABLE";
            break;
        case BLE_GAP_SEC_STATUS_AUTH_REQ:
            retval << "BLE_GAP_SEC_STATUS_AUTH_REQ";
            break;
        case BLE_GAP_SEC_STATUS_CONFIRM_VALUE:
            retval << "BLE_GAP_SEC_STATUS_CONFIRM_VALUE";
            break;
        case BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP:
            retval << "BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP";
            break;
        case BLE_GAP_SEC_STATUS_ENC_KEY_SIZE:
            retval << "BLE_GAP_SEC_STATUS_ENC_KEY_SIZE";
            break;
        case BLE_GAP_SEC_STATUS_SMP_CMD_UNSUPPORTED:
            retval << "BLE_GAP_SEC_STATUS_SMP_CMD_UNSUPPORTED";
            break;
        case BLE_GAP_SEC_STATUS_UNSPECIFIED:
            retval << "BLE_GAP_SEC_STATUS_UNSPECIFIED";
            break;
        case BLE_GAP_SEC_STATUS_REPEATED_ATTEMPTS:
            retval << "BLE_GAP_SEC_STATUS_REPEATED_ATTEMPTS";
            break;
        case BLE_GAP_SEC_STATUS_INVALID_PARAMS:
            retval << "BLE_GAP_SEC_STATUS_INVALID_PARAMS";
            break;
        case BLE_GAP_SEC_STATUS_DHKEY_FAILURE:
            retval << "BLE_GAP_SEC_STATUS_DHKEY_FAILURE";
            break;
        case BLE_GAP_SEC_STATUS_NUM_COMP_FAILURE:
            retval << "BLE_GAP_SEC_STATUS_NUM_COMP_FAILURE";
            break;
        case BLE_GAP_SEC_STATUS_BR_EDR_IN_PROG:
            retval << "BLE_GAP_SEC_STATUS_BR_EDR_IN_PROG";
            break;
        case BLE_GAP_SEC_STATUS_X_TRANS_KEY_DISALLOWED:
            retval << "BLE_GAP_SEC_STATUS_X_TRANS_KEY_DISALLOWED";
            break;
        case BLE_GAP_SEC_STATUS_RFU_RANGE2_BEGIN:
            retval << "BLE_GAP_SEC_STATUS_RFU_RANGE2_BEGIN";
            break;
        case BLE_GAP_SEC_STATUS_RFU_RANGE2_END:
            retval << "BLE_GAP_SEC_STATUS_RFU_RANGE2_END";
            break;
    }

    retval << "/0x" << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)authStatus << ".";
    return retval.str();
}

static std::string asText(const ble_gap_evt_passkey_display_t &passkeyDisplay) {
    std::stringstream retval;
    std::stringstream passkey;

    for (auto i = 0; i < 6; i++)
    {
        passkey << static_cast<char>(passkeyDisplay.passkey[i]);
    }

    retval << "passkey: '" << passkey.str() << "'";
    retval << " match_request:" << (passkeyDisplay.match_request ? "yes" : "no");
    return retval.str();
}

static std::string asText(const ble_gap_sec_kdist_t &keyDist) {
    std::stringstream retval;
    retval << "enc:" << (keyDist.enc ? "yes" : "no") << " ";
    retval << "id:" << (keyDist.id ? "yes" : "no") << " ";
    retval << "sign:" << (keyDist.sign ? "yes" : "no") << " ";
    retval << "link:" << (keyDist.link ? "yes" : "no");
    return retval.str();
}

static std::string asText(const ble_gap_evt_sec_request_t &securityRequest) {
    std::stringstream retval;
    retval << "bond:" << (securityRequest.bond ? "yes" : "no") << " ";
    retval << "mitm:" << (securityRequest.mitm ? "yes" : "no") << " ";
    retval << "lesc:" << (securityRequest.lesc ? "yes" : "no") << " ";
    retval << "keypress:" << (securityRequest.keypress ? "yes" : "no");
    return retval.str();
}

static std::string asText(const ble_gap_evt_sec_params_request_t &securityParamsRequest) {
    std::stringstream retval;
    retval << "peer_params:[";
    retval << "bond:" << (securityParamsRequest.peer_params.bond ? "yes" : "no") << " ";
    retval << "mitm:" << (securityParamsRequest.peer_params.mitm ? "yes" : "no") << " ";
    retval << "lesc:" << (securityParamsRequest.peer_params.lesc ? "yes" : "no") << " ";
    retval << "keypress:" << (securityParamsRequest.peer_params.keypress ? "yes" : "no") << " ";
    retval << "io_caps:" << testutil::ioCapsToString(securityParamsRequest.peer_params.io_caps)
           << " ";
    retval << "oob:" << (securityParamsRequest.peer_params.oob ? "yes" : "no") << " ";
    retval << "min_key_size:"
           << static_cast<uint32_t>(securityParamsRequest.peer_params.min_key_size) << " ";
    retval << "max_key_size:"
           << static_cast<uint32_t>(securityParamsRequest.peer_params.max_key_size) << " ";
    retval << "kdist_own:[" << asText(securityParamsRequest.peer_params.kdist_own) << "] ";
    retval << "kdist_peer:[" << asText(securityParamsRequest.peer_params.kdist_peer) << "]";
    retval << "]";
    return retval.str();
}

static std::string asText(const ble_gap_evt_auth_key_request_t &authKeyRequest) {
    std::stringstream retval;
    retval << "key_type: ";

    switch (authKeyRequest.key_type)
    {
        case BLE_GAP_AUTH_KEY_TYPE_NONE:
            retval << "BLE_GAP_AUTH_KEY_TYPE_NONE";
            break;
        case BLE_GAP_AUTH_KEY_TYPE_PASSKEY:
            retval << "BLE_GAP_AUTH_KEY_TYPE_PASSKEY";
            break;
        case BLE_GAP_AUTH_KEY_TYPE_OOB:
            retval << "BLE_GAP_AUTH_KEY_TYPE_OOB";
            break;
        default:
            retval << asHex(authKeyRequest.key_type);
            break;
    }
    return retval.str();
}

static std::string asText(const ble_gap_conn_sec_mode_t &connSecMode) {
    std::stringstream retval;

    retval << "sm:" << static_cast<uint32_t>(connSecMode.sm)
           << " lv:" << static_cast<uint32_t>(connSecMode.lv) << " description:'";

    if (connSecMode.sm == 0 && connSecMode.lv == 0)
    {
        retval << "Security Mode 0 Level 0: No access permissions at all";
    }
    else if (connSecMode.sm == 1 && connSecMode.lv == 1)
    { retval << "No security is needed (aka open link)"; }
    else if (connSecMode.sm == 1 && connSecMode.lv == 2)
    { retval << "Encrypted link required, MITM protection not necessary"; }
    else if (connSecMode.sm == 1 && connSecMode.lv == 3)
    { retval << "MITM protected encrypted link required"; }
    else if (connSecMode.sm == 1 && connSecMode.lv == 4)
    { retval << "LESC MITM protected encrypted link required"; }
    else if (connSecMode.sm == 2 && connSecMode.lv == 1)
    { retval << "Signing or encryption required, MITM protection not necessary"; }
    else if (connSecMode.sm == 2 && connSecMode.lv == 2)
    { retval << "MITM protected signing required, unless link is MITM protected encrypted"; }

    retval << "'";
    return retval.str();
}

static std::string asText(const ble_gap_conn_sec_t &connSec) {
    std::stringstream retval;
    retval << "sec_mode:[" << asText(connSec.sec_mode) << "] ";
    retval << "encr_key_size:" << static_cast<uint32_t>(connSec.encr_key_size);
    return retval.str();
}

static std::string asText(const ble_gap_evt_conn_sec_update_t &connSecUpdate) {
    std::stringstream retval;
    retval << "conn_sec:[";
    retval << asText(connSecUpdate.conn_sec);
    retval << "]";
    return retval.str();
}

static std::string asText(const ble_gap_sec_levels_t &secLevels) {
    std::stringstream retval;
    retval << "lv1:" << (secLevels.lv1 ? "yes" : "no");
    retval << " lv2:" << (secLevels.lv2 ? "yes" : "no");
    retval << " lv3:" << (secLevels.lv3 ? "yes" : "no");
    retval << " lv4:" << (secLevels.lv4 ? "yes" : "no");
    return retval.str();
}

static std::string asText(const ble_gap_evt_auth_status_t &authStatus) {
    std::stringstream retval;
    retval << "auth_status:" << gattAuthStatusToString(authStatus.auth_status) << " ";
    retval << "error_src:" << gattAuthErrorSrcToString(authStatus.error_src) << " ";
    retval << "bonded:" << (authStatus.bonded ? "yes" : "no") << " ";
    retval << "sm1_levels:[" << asText(authStatus.sm1_levels) << "] ";
    retval << "sm2_levels:[" << asText(authStatus.sm2_levels) << "] ";
    retval << "kdist_own:[" << asText(authStatus.kdist_own) << "] ";
    retval << "kdist_peer:[" << asText(authStatus.kdist_peer) << "]";
    return retval.str();
}

/**
 * @brief Function that convert a GATT status code to string representation
 *
 * param[in] code code
 *
 * @return textual representation of the code
 */
static std::string gattStatusToString(const uint16_t code) {
    std::stringstream retval;
    retval << "GATT status code: 0x" << std::setfill('0') << std::setw(4) << std::hex
           << (uint32_t)code << ".";
    return retval.str();
}

// Operator overloading for common types used
std::string asText(const sd_rpc_app_status_t &status) {
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

static std::string asText(const uint16_t &value) {
    std::stringstream retval;
    retval << "0x" << std::setfill('0') << std::setw(4) << std::hex << (uint32_t)value;
    return retval.str();
}

static std::string asText(const uint8_t &value) {
    std::stringstream retval;
    retval << "0x" << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)value;
    return retval.str();
}

static std::string asText(const ble_uuid_t &uuid) {
    std::stringstream retval;
    retval << "uuid:[type: " << asText(uuid.type) << ", uuid: " << asText(uuid.uuid) << "]";
    return retval.str();
}

static std::string asText(const ble_gattc_char_t &gattc_char) {
    std::stringstream retval;
    retval << "characteristic:["
           << "handle_decl: " << asText(gattc_char.handle_decl)
           << ", handle_value: " << asText(gattc_char.handle_value)
           << ", uuid: " << asText(gattc_char.uuid) << "]";

    return retval.str();
}

static std::string asText(const ble_gattc_handle_range_t &gattc_handle_range) {
    std::stringstream retval;
    retval << "handle range:["
           << "start: " << asText(gattc_handle_range.start_handle)
           << ", end: " << asText(gattc_handle_range.end_handle) << "]";
    return retval.str();
}

static std::string asText(const ble_gattc_desc_t &gattc_desc) {
    std::stringstream retval;
    retval << "descriptor:["
           << "handle: " << asText(gattc_desc.handle) << " " << asText(gattc_desc.uuid) << "]";
    return retval.str();
}

static std::string asText(const sd_rpc_log_severity_t &severity) {
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
