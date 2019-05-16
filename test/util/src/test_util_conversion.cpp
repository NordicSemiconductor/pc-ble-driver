#include "test_util_conversion.h"

#include "ble.h"
#include "ble_hci.h"
#include "sd_rpc_types.h"

#include <spdlog/logger.h>

#include <cctype>
#include <iomanip>
#include <map>
#include <sstream>
#include <vector>

#define NAME_MAP_ENTRY(enumerator)                                                                 \
    {                                                                                              \
        enumerator, "" #enumerator ""                                                              \
    }

namespace testutil {
/**
 * @brief Function that convert data to string representation
 * @param[in] data Data to represent as a string
 * @return string representation of data
 */
std::string asHex(const std::vector<uint8_t> &data)
{
    std::stringstream retval;

    for (uint8_t const &value : data)
    {
        retval << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(value) << ' ';
    }

    return retval.str();
}

std::string asHex(const uint16_t &data)
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
std::string asText(const ble_gap_addr_t &address)
{
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
std::string errorToString(const uint32_t error_code)
{
    std::stringstream retval;
    retval << "error code: 0x" << std::setfill('0') << std::setw(2) << std::hex << error_code;
    return retval.str();
}

std::string ioCapsToString(const uint8_t code)
{
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

std::string roleToString(const uint8_t role)
{
    std::stringstream retval;

    switch (role)
    {
        case BLE_GAP_ROLE_INVALID:
            retval << "BLE_GAP_ROLE_INVALID";
            break;
        case BLE_GAP_ROLE_PERIPH:
            retval << "BLE_GAP_ROLE_PERIPH";
            break;
        case BLE_GAP_ROLE_CENTRAL:
            retval << "BLE_GAP_ROLE_CENTRAL";
            break;
        default:
            retval << "UNKNOWN";
            break;
    }

    return retval.str();
}

std::string gattAuthErrorSrcToString(const uint8_t errorSrc)
{
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

    retval << "/0x" << std::setfill('0') << std::setw(2) << std::hex
           << static_cast<uint32_t>(errorSrc) << ".";
    return retval.str();
}

std::string gattAuthStatusToString(const uint8_t authStatus)
{
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
        default:
            break;
    }

    retval << "/0x" << std::setfill('0') << std::setw(2) << std::hex
           << static_cast<uint32_t>(authStatus) << ".";
    return retval.str();
}

std::string hciStatusCodeToString(const uint8_t hciStatusCode)
{
    std::stringstream retval;

    switch (hciStatusCode)
    {
        case BLE_HCI_STATUS_CODE_SUCCESS:
            retval << "BLE_HCI_STATUS_CODE_SUCCESS";
            break;
        case BLE_HCI_STATUS_CODE_UNKNOWN_BTLE_COMMAND:
            retval << "BLE_HCI_STATUS_CODE_UNKNOWN_BTLE_COMMAND";
            break;
        case BLE_HCI_STATUS_CODE_UNKNOWN_CONNECTION_IDENTIFIER:
            retval << "BLE_HCI_STATUS_CODE_UNKNOWN_CONNECTION_IDENTIFIER";
            break;
        case BLE_HCI_AUTHENTICATION_FAILURE:
            retval << "BLE_HCI_AUTHENTICATION_FAILURE";
            break;
        case BLE_HCI_STATUS_CODE_PIN_OR_KEY_MISSING:
            retval << "BLE_HCI_STATUS_CODE_PIN_OR_KEY_MISSING";
            break;
        case BLE_HCI_MEMORY_CAPACITY_EXCEEDED:
            retval << "BLE_HCI_MEMORY_CAPACITY_EXCEEDED";
            break;
        case BLE_HCI_CONNECTION_TIMEOUT:
            retval << "BLE_HCI_CONNECTION_TIMEOUT";
            break;
        case BLE_HCI_STATUS_CODE_COMMAND_DISALLOWED:
            retval << "BLE_HCI_STATUS_CODE_COMMAND_DISALLOWED";
            break;
        case BLE_HCI_STATUS_CODE_INVALID_BTLE_COMMAND_PARAMETERS:
            retval << "BLE_HCI_STATUS_CODE_INVALID_BTLE_COMMAND_PARAMETERS";
            break;
        case BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION:
            retval << "BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION";
            break;
        case BLE_HCI_REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES:
            retval << "BLE_HCI_REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES";
            break;
        case BLE_HCI_REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF:
            retval << "BLE_HCI_REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF";
            break;
        case BLE_HCI_LOCAL_HOST_TERMINATED_CONNECTION:
            retval << "BLE_HCI_LOCAL_HOST_TERMINATED_CONNECTION";
            break;
        case BLE_HCI_UNSUPPORTED_REMOTE_FEATURE:
            retval << "BLE_HCI_UNSUPPORTED_REMOTE_FEATURE";
            break;
        case BLE_HCI_STATUS_CODE_INVALID_LMP_PARAMETERS:
            retval << "BLE_HCI_STATUS_CODE_INVALID_LMP_PARAMETERS";
            break;
        case BLE_HCI_STATUS_CODE_UNSPECIFIED_ERROR:
            retval << "BLE_HCI_STATUS_CODE_UNSPECIFIED_ERROR";
            break;
        case BLE_HCI_STATUS_CODE_LMP_RESPONSE_TIMEOUT:
            retval << "BLE_HCI_STATUS_CODE_LMP_RESPONSE_TIMEOUT";
            break;
#if NRF_SD_BLE_API > 3
        case BLE_HCI_STATUS_CODE_LMP_ERROR_TRANSACTION_COLLISION:
            retval << "BLE_HCI_STATUS_CODE_LMP_ERROR_TRANSACTION_COLLISION";
            break;
#endif
        case BLE_HCI_STATUS_CODE_LMP_PDU_NOT_ALLOWED:
            retval << "BLE_HCI_STATUS_CODE_LMP_PDU_NOT_ALLOWED";
            break;
        case BLE_HCI_INSTANT_PASSED:
            retval << "BLE_HCI_INSTANT_PASSED";
            break;
        case BLE_HCI_PAIRING_WITH_UNIT_KEY_UNSUPPORTED:
            retval << "BLE_HCI_PAIRING_WITH_UNIT_KEY_UNSUPPORTED";
            break;
        case BLE_HCI_DIFFERENT_TRANSACTION_COLLISION:
            retval << "BLE_HCI_DIFFERENT_TRANSACTION_COLLISION";
            break;
#if NRF_SD_BLE_API > 3
        case BLE_HCI_PARAMETER_OUT_OF_MANDATORY_RANGE:
            retval << "BLE_HCI_PARAMETER_OUT_OF_MANDATORY_RANGE";
            break;
#endif
        case BLE_HCI_CONTROLLER_BUSY:
            retval << "BLE_HCI_CONTROLLER_BUSY";
            break;
        case BLE_HCI_CONN_INTERVAL_UNACCEPTABLE:
            retval << "BLE_HCI_CONN_INTERVAL_UNACCEPTABLE";
            break;
        case BLE_HCI_DIRECTED_ADVERTISER_TIMEOUT:
            retval << "BLE_HCI_DIRECTED_ADVERTISER_TIMEOUT";
            break;
        case BLE_HCI_CONN_TERMINATED_DUE_TO_MIC_FAILURE:
            retval << "BLE_HCI_CONN_TERMINATED_DUE_TO_MIC_FAILURE";
            break;
        case BLE_HCI_CONN_FAILED_TO_BE_ESTABLISHED:
            retval << "BLE_HCI_CONN_FAILED_TO_BE_ESTABLISHED";
            break;
        default:
            retval << "UNKNOWN";
            break;
    }

    return retval.str();
}

std::string asText(const ble_gap_evt_passkey_display_t &passkeyDisplay)
{
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

std::string asText(const ble_gap_enc_info_t &encryptionInfo)
{
    std::stringstream retval;
    std::vector<uint8_t> key(encryptionInfo.ltk, encryptionInfo.ltk + encryptionInfo.ltk_len);

    retval << "ltk:" << asHex(key) << " ";
    retval << "lesc:" << (encryptionInfo.lesc ? "yes" : "no") << " ";
    retval << "auth:" << (encryptionInfo.auth ? "yes" : "no") << " ";
    retval << "ltk_len:" << static_cast<uint32_t>(encryptionInfo.ltk_len);
    return retval.str();
}

std::string asText(const ble_gap_conn_params_t &connectionParams)
{
    std::stringstream retval;

    retval << "conn_sup_timeout:" << static_cast<uint32_t>(connectionParams.conn_sup_timeout)
           << " ";
    retval << "max_conn_interval:" << static_cast<uint32_t>(connectionParams.max_conn_interval)
           << " ";
    retval << "min_conn_interval:" << static_cast<uint32_t>(connectionParams.min_conn_interval)
           << " ";
    retval << "slave_latency:" << static_cast<uint32_t>(connectionParams.slave_latency);

    return retval.str();
}

std::string asText(const ble_gap_evt_connected_t &connected)
{
    std::stringstream retval;

    retval << "peer_addr:[" << asText(connected.peer_addr) << "] ";
#if NRF_SD_BLE_API == 2
    retval << "own_addr:[" << asText(connected.own_addr) << "] ";
#endif // NRF_SD_BLE_API == 2
    retval << "role:" << roleToString(connected.role) << " ";
#if NRF_SD_BLE_API == 2
    retval << "irk_match:" << (connected.irk_match ? "yes" : "no") << " ";
    retval << "irk_match_idx:" << static_cast<uint8_t>(connected.irk_match_idx) << " ";
#endif // NRF_SD_BLE_API == 2
    retval << "conn_params:[" << asText(connected.conn_params) << "]";
    return retval.str();
}

std::string asText(const ble_gap_evt_disconnected_t &disconnected)
{
    std::stringstream retval;

    retval << "reason:" << hciStatusCodeToString(disconnected.reason);
    return retval.str();
}

std::string asText(const ble_gap_evt_scan_req_report_t &scanReqReport)
{
    std::stringstream retval;

    retval << "peer_addr:[" << asText(scanReqReport.peer_addr) << "]";
    retval << " rssi:" << static_cast<int32_t>(scanReqReport.rssi);
    return retval.str();
}

std::string asText(const ble_gap_sec_kdist_t &keyDist)
{
    std::stringstream retval;
    retval << "enc:" << (keyDist.enc ? "yes" : "no") << " ";
    retval << "id:" << (keyDist.id ? "yes" : "no") << " ";
    retval << "sign:" << (keyDist.sign ? "yes" : "no") << " ";
    retval << "link:" << (keyDist.link ? "yes" : "no");
    return retval.str();
}

std::string asText(const ble_gap_evt_sec_request_t &securityRequest)
{
    std::stringstream retval;
    retval << "bond:" << (securityRequest.bond ? "yes" : "no") << " ";
    retval << "mitm:" << (securityRequest.mitm ? "yes" : "no") << " ";
    retval << "lesc:" << (securityRequest.lesc ? "yes" : "no") << " ";
    retval << "keypress:" << (securityRequest.keypress ? "yes" : "no");
    return retval.str();
}

std::string asText(const ble_gap_evt_sec_params_request_t &securityParamsRequest)
{
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

std::string asText(const ble_gap_evt_auth_key_request_t &authKeyRequest)
{
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
            retval << "0x" << asHex(authKeyRequest.key_type);
            break;
    }
    return retval.str();
}

std::string asText(const ble_gap_conn_sec_mode_t &connSecMode)
{
    std::stringstream retval;

    retval << "sm:" << static_cast<uint32_t>(connSecMode.sm)
           << " lv:" << static_cast<uint32_t>(connSecMode.lv) << " description:'";

    if (connSecMode.sm == 0 && connSecMode.lv == 0)
    {
        retval << "Security Mode 0 Level 0: No access permissions at all";
    }
    else if (connSecMode.sm == 1 && connSecMode.lv == 1)
    {
        retval << "No security is needed (aka open link)";
    }
    else if (connSecMode.sm == 1 && connSecMode.lv == 2)
    {
        retval << "Encrypted link required, MITM protection not necessary";
    }
    else if (connSecMode.sm == 1 && connSecMode.lv == 3)
    {
        retval << "MITM protected encrypted link required";
    }
    else if (connSecMode.sm == 1 && connSecMode.lv == 4)
    {
        retval << "LESC MITM protected encrypted link required";
    }
    else if (connSecMode.sm == 2 && connSecMode.lv == 1)
    {
        retval << "Signing or encryption required, MITM protection not necessary";
    }
    else if (connSecMode.sm == 2 && connSecMode.lv == 2)
    {
        retval << "MITM protected signing required, unless link is MITM protected encrypted";
    }

    retval << "'";
    return retval.str();
}

std::string asText(const ble_gap_conn_sec_t &connSec)
{
    std::stringstream retval;
    retval << "sec_mode:[" << asText(connSec.sec_mode) << "] ";
    retval << "encr_key_size:" << static_cast<uint32_t>(connSec.encr_key_size);
    return retval.str();
}

std::string asText(const ble_gap_evt_rssi_changed_t &rssiChanged)
{
    std::stringstream retval;
    retval << "rssi:" << static_cast<int32_t>(rssiChanged.rssi) << "dBm";
#if NRF_SD_BLE_API >= 6
    retval << " ch_index:" << static_cast<uint32_t>(rssiChanged.ch_index);
#endif
    return retval.str();
}

std::string asText(const ble_gap_lesc_p256_pk_t &lsecP256Pk)
{
    std::stringstream retval;
    std::vector<uint8_t> key(lsecP256Pk.pk, lsecP256Pk.pk + 64);
    retval << "key:" << asHex(key);
    return retval.str();
}

std::string asText(const ble_gap_evt_key_pressed_t &keyPressed)
{
    std::stringstream retval;
    retval << "pk_not:" << keyPressed.kp_not; // TODO: convert to string
    return retval.str();
}

std::string asText(const std::vector<uint8_t> &data)
{
    std::stringstream retval;

    for (auto v : data)
    {
        if (std::isprint(v))
        {
            retval << v;
        }
    }

    return retval.str();
}

std::string asText(const ble_gap_evt_timeout_t &timeout)
{
    std::stringstream retval;
    retval << "src:" << timeout.src; // TODO: convert to string
    return retval.str();
}

#if NRF_SD_BLE_API == 6
std::string asText(const ble_gap_adv_report_type_t &reportType)
{
    std::stringstream retval;

    if (reportType.connectable)
    {
        retval << " connectable:" << (reportType.connectable ? "yes" : "no");
    }

    if (reportType.scannable)
    {
        retval << " scannable:" << (reportType.scannable ? "yes" : "no");
    }

    if (reportType.directed)
    {
        retval << " directed:" << (reportType.directed ? "yes" : "no");
    }

    if (reportType.scan_response)
    {
        retval << " scan_response:" << (reportType.scan_response ? "yes" : "no");
    }

    if (reportType.extended_pdu)
    {
        retval << " extended_pdu:" << (reportType.extended_pdu ? "yes" : "no");
    }

    retval << " status:";

    switch (reportType.status)
    {
        case BLE_GAP_ADV_DATA_STATUS_COMPLETE:
            retval << "COMPLETE";
            break;
        case BLE_GAP_ADV_DATA_STATUS_INCOMPLETE_MORE_DATA:
            retval << "INCOMPLETE_MORE_DATA";
            break;
        case BLE_GAP_ADV_DATA_STATUS_INCOMPLETE_TRUNCATED:
            retval << "INCOMPLETE_TRUNCATED";
            break;
        case BLE_GAP_ADV_DATA_STATUS_INCOMPLETE_MISSED:
            retval << "INCOMPLETE_MISSED";
            break;
        default:
            retval << "UNKNOWN(" << std::hex << static_cast<uint32_t>(reportType.status) << ")";
            break;
    }

    return retval.str();
}

std::string asText(const ble_gap_aux_pointer_t &auxPointer)
{
    std::stringstream retval;
    retval << "aux_offset:" << static_cast<uint32_t>(auxPointer.aux_offset);
    retval << " aux_phy:" << static_cast<uint32_t>(auxPointer.aux_phy);
    return retval.str();
}

std::string asText(const ble_gap_phys_t &phys)
{
    std::stringstream retval;
    retval << "tx_phys:" << static_cast<uint32_t>(phys.tx_phys);
    retval << " rx_phys:" << static_cast<uint32_t>(phys.rx_phys);
    return retval.str();
}

std::string asText(const ble_gap_evt_phy_update_request_t &phyUpdateRequest)
{
    std::stringstream retval;
    retval << "peer_preferred_phys:[" << asText(phyUpdateRequest.peer_preferred_phys) << "]";
    return retval.str();
}

std::string asText(const ble_gap_evt_phy_update_t &phyUpdate)
{
    std::stringstream retval;
    retval << "status: " << static_cast<uint32_t>(phyUpdate.status);
    retval << " tx_phy:" << static_cast<uint32_t>(phyUpdate.tx_phy);
    retval << " rx_phy:" << static_cast<uint32_t>(phyUpdate.rx_phy);
    return retval.str();
}

std::string asText(const ble_gap_data_length_params_t &dataLengthParams)
{
    std::stringstream retval;
    retval << "max_tx_octets: " << static_cast<uint32_t>(dataLengthParams.max_tx_octets);
    retval << "max_rx_octets: " << static_cast<uint32_t>(dataLengthParams.max_rx_octets);
    retval << "max_tx_time_us: " << static_cast<uint32_t>(dataLengthParams.max_tx_time_us);
    retval << "max_rx_time_us: " << static_cast<uint32_t>(dataLengthParams.max_rx_time_us);
    return retval.str();
}

std::string asText(const ble_gap_evt_data_length_update_request_t &dataLengthUpdateRequest)
{
    std::stringstream retval;
    retval << "peer_params:[" << asText(dataLengthUpdateRequest.peer_params) << "]";
    return retval.str();
}

std::string asText(const ble_gap_evt_data_length_update_t &dataLengthUpdate)
{
    std::stringstream retval;
    retval << "effective_params:[" << asText(dataLengthUpdate.effective_params) << "]";
    return retval.str();
}

std::string asText(const ble_gap_evt_qos_channel_survey_report_t &qosChannelSurveyReport)
{
    std::stringstream retval;
    std::vector<uint8_t> channels;
    channels.assign(qosChannelSurveyReport.channel_energy,
                    qosChannelSurveyReport.channel_energy + BLE_GAP_CHANNEL_COUNT);

    retval << "channel_energy:";
    for (auto channel : channels)
    {
        retval << " " << static_cast<uint32_t>(channel);
    }

    return retval.str();
}

std::string asText(const ble_data_t &data)
{
    std::stringstream retval;

    std::vector<uint8_t> wrappedData;

    if (data.p_data != nullptr)
    {
        wrappedData.assign(data.p_data, data.p_data + data.len);
        retval << "data:" << asHex(wrappedData);
        retval << " len:" << static_cast<uint32_t>(data.len);
    }
    else
    {
        retval << "data:null";
        retval << " len:" << static_cast<uint32_t>(data.len);
    }

    return retval.str();
}

std::string asText(const ble_gap_adv_data_t &advData)
{
    std::stringstream retval;

    retval << "adv_data:[" << asText(advData.adv_data) << "]";
    retval << " scan_rsp_data:[" << asText(advData.scan_rsp_data) << "]";
    return retval.str();
}

std::string asText(const ble_gap_evt_adv_set_terminated_t &advSetTerminated)
{
    std::stringstream retval;
    retval << "reason:" << static_cast<uint32_t>(advSetTerminated.reason);
    retval << " adv_handle:" << asHex(advSetTerminated.adv_handle);
    retval << " num_completed_adv_events:"
           << static_cast<uint32_t>(advSetTerminated.num_completed_adv_events);
    retval << " adv_data:[" << asText(advSetTerminated.adv_data) << "]";

    return retval.str();
}

#endif // NRF_SD_BLE_API == 6

std::string asText(const ble_gap_evt_adv_report_t &advReport)
{
    std::stringstream retval;

    retval << "peer_addr:[" << asText(advReport.peer_addr) << "]";
    std::vector<uint8_t> data;

#if NRF_SD_BLE_API == 6
    if (advReport.data.p_data != nullptr)
    {
        data =
            std::vector<uint8_t>(advReport.data.p_data, advReport.data.p_data + advReport.data.len);
        retval << " type:[" << asText(advReport.type) << "]";
    }
#else
    data = std::vector<uint8_t>(advReport.data, advReport.data + advReport.dlen);
    retval << " type:" << asHex(advReport.type);
#endif

#if NRF_SD_BLE_API == 6
    if (advReport.type.directed)
    {
        retval << " direct_addr:[" << asText(advReport.direct_addr) << "]";
    }

    retval << " primary_phy:" << static_cast<uint32_t>(advReport.primary_phy);
    retval << " secondary_phy:" << static_cast<uint32_t>(advReport.secondary_phy);

    if (advReport.tx_power != BLE_GAP_POWER_LEVEL_INVALID)
    {
        retval << " tx_power:" << static_cast<uint32_t>(advReport.tx_power);
    }
#endif

    retval << " rssi:" << static_cast<int32_t>(advReport.rssi) << "dBm";

#if NRF_SD_BLE_API == 6
    retval << " ch_index:" << std::hex << static_cast<uint32_t>(advReport.ch_index);

    if (advReport.set_id != BLE_GAP_ADV_REPORT_SET_ID_NOT_AVAILABLE)
    {
        retval << " set_id:" << std::hex << static_cast<uint32_t>(advReport.set_id);
    }

    if (advReport.data_id != BLE_GAP_ADV_REPORT_SET_ID_NOT_AVAILABLE)
    {
        retval << " data_id:" << std::hex << static_cast<uint32_t>(advReport.data_id);
    }

    if (advReport.type.status == BLE_GAP_ADV_DATA_STATUS_INCOMPLETE_MORE_DATA)
    {
        retval << " aux_pointer:[" << asText(advReport.aux_pointer) << "]";
    }
#endif

#if NRF_SD_BLE_API < 6
    retval << " scan_rsp:" << (advReport.scan_rsp ? "yes" : "no");
#endif

    retval << " data:" << asHex(data) << "/" << asText(data);

    return retval.str();
}

std::string asText(const ble_gap_evt_lesc_dhkey_request_t &dhkeyRequest)
{
    std::stringstream retval;
    retval << "p_pk_peer:["
           << ((dhkeyRequest.p_pk_peer != nullptr) ? asText(*dhkeyRequest.p_pk_peer) : "NULL")
           << "]";
    retval << " oobd_req:" << (dhkeyRequest.oobd_req ? "yes" : "no");
    return retval.str();
}

std::string asText(const ble_gap_master_id_t &masterId)
{
    std::stringstream retval;
    std::vector<uint8_t> rand(masterId.rand, masterId.rand + sizeof(masterId.rand));
    retval << "ediv:0x" << asHex(masterId.ediv);
    retval << " rand:0x" << asHex(rand);
    return retval.str();
}

std::string asText(const ble_gap_evt_sec_info_request_t &secInfoRequest)
{
    std::stringstream retval;
    retval << "peer_addr:[" << asText(secInfoRequest.peer_addr) << "]";
    retval << " enc_info:" << (secInfoRequest.enc_info ? "required" : "not_required");
    retval << " id_info:"
           << (secInfoRequest.id_info ? "identity_information_required"
                                      : "identity_information_not_required");
    retval << " master_id:[" << asText(secInfoRequest.master_id) << "]";
    retval << " sign_info:" << secInfoRequest.sign_info
           << (secInfoRequest.sign_info ? "signing_information_required"
                                        : "signing_information_not_required");
    return retval.str();
}

std::string asText(const ble_gap_evt_conn_sec_update_t &connSecUpdate)
{
    std::stringstream retval;
    retval << "conn_sec:[";
    retval << asText(connSecUpdate.conn_sec);
    retval << "]";
    return retval.str();
}

std::string asText(const ble_gap_sec_levels_t &secLevels)
{
    std::stringstream retval;
    retval << "lv1:" << (secLevels.lv1 ? "yes" : "no");
    retval << " lv2:" << (secLevels.lv2 ? "yes" : "no");
    retval << " lv3:" << (secLevels.lv3 ? "yes" : "no");
    retval << " lv4:" << (secLevels.lv4 ? "yes" : "no");
    return retval.str();
}

std::string asText(const ble_gap_evt_auth_status_t &authStatus)
{
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
std::string gattStatusToString(const uint16_t code)
{
    std::stringstream retval;
    retval << "GATT status code: 0x" << std::setfill('0') << std::setw(4) << std::hex
           << (uint32_t)code << ".";
    return retval.str();
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

std::string asText(const uint16_t &value)
{
    std::stringstream retval;
    retval << "0x" << std::setfill('0') << std::setw(4) << std::hex << (uint32_t)value;
    return retval.str();
}

std::string asText(const uint8_t &value)
{
    std::stringstream retval;
    retval << "0x" << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)value;
    return retval.str();
}

std::string asText(const ble_uuid_t &uuid)
{
    std::stringstream retval;
    retval << "uuid:[type: " << asText(uuid.type) << ", uuid: " << asText(uuid.uuid) << "]";
    return retval.str();
}

std::string asText(const ble_gattc_char_t &gattc_char)
{
    std::stringstream retval;
    retval << "characteristic:["
           << "handle_decl: " << asText(gattc_char.handle_decl)
           << ", handle_value: " << asText(gattc_char.handle_value)
           << ", uuid: " << asText(gattc_char.uuid) << "]";

    return retval.str();
}

std::string asText(const ble_gattc_handle_range_t &gattc_handle_range)
{
    std::stringstream retval;
    retval << "handle range:["
           << "start: " << asText(gattc_handle_range.start_handle)
           << ", end: " << asText(gattc_handle_range.end_handle) << "]";
    return retval.str();
}

std::string asText(const ble_gattc_desc_t &gattc_desc)
{
    std::stringstream retval;
    retval << "descriptor:["
           << "handle: " << asText(gattc_desc.handle) << " " << asText(gattc_desc.uuid) << "]";
    return retval.str();
}

std::string eventIdAsText(const uint32_t eventId)
{
    static std::map<uint32_t, std::string> eventIds = {
        {// GAP Events
         NAME_MAP_ENTRY(BLE_GAP_EVT_CONNECTED),
         NAME_MAP_ENTRY(BLE_GAP_EVT_DISCONNECTED),
         NAME_MAP_ENTRY(BLE_GAP_EVT_CONN_PARAM_UPDATE),
         NAME_MAP_ENTRY(BLE_GAP_EVT_SEC_PARAMS_REQUEST),
         NAME_MAP_ENTRY(BLE_GAP_EVT_SEC_INFO_REQUEST),
         NAME_MAP_ENTRY(BLE_GAP_EVT_PASSKEY_DISPLAY),
         NAME_MAP_ENTRY(BLE_GAP_EVT_KEY_PRESSED),
         NAME_MAP_ENTRY(BLE_GAP_EVT_AUTH_KEY_REQUEST),
         NAME_MAP_ENTRY(BLE_GAP_EVT_LESC_DHKEY_REQUEST),
         NAME_MAP_ENTRY(BLE_GAP_EVT_AUTH_STATUS),
         NAME_MAP_ENTRY(BLE_GAP_EVT_CONN_SEC_UPDATE),
         NAME_MAP_ENTRY(BLE_GAP_EVT_TIMEOUT),
         NAME_MAP_ENTRY(BLE_GAP_EVT_RSSI_CHANGED),
         NAME_MAP_ENTRY(BLE_GAP_EVT_ADV_REPORT),
         NAME_MAP_ENTRY(BLE_GAP_EVT_SEC_REQUEST),
         NAME_MAP_ENTRY(BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST),
         NAME_MAP_ENTRY(BLE_GAP_EVT_SCAN_REQ_REPORT),
#if NRF_SD_BLE_API >= 6
         NAME_MAP_ENTRY(BLE_GAP_EVT_PHY_UPDATE_REQUEST),
         NAME_MAP_ENTRY(BLE_GAP_EVT_PHY_UPDATE),
         NAME_MAP_ENTRY(BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST),
         NAME_MAP_ENTRY(BLE_GAP_EVT_DATA_LENGTH_UPDATE),
         NAME_MAP_ENTRY(BLE_GAP_EVT_QOS_CHANNEL_SURVEY_REPORT),
         NAME_MAP_ENTRY(BLE_GAP_EVT_ADV_SET_TERMINATED),
#endif // NRF_SD_BLE_API >= 6
       // GATTS events
         NAME_MAP_ENTRY(BLE_GATTS_EVT_WRITE),
         NAME_MAP_ENTRY(BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST),
         NAME_MAP_ENTRY(BLE_GATTS_EVT_SYS_ATTR_MISSING),
         NAME_MAP_ENTRY(BLE_GATTS_EVT_HVC),
         NAME_MAP_ENTRY(BLE_GATTS_EVT_SC_CONFIRM),
         NAME_MAP_ENTRY(BLE_GATTS_EVT_TIMEOUT),
#if NRF_SD_BLE_API >= 5
         NAME_MAP_ENTRY(BLE_GATTS_EVT_HVN_TX_COMPLETE),
#endif // NRF_SD_BLE_API >= 5
       // GATTS events
         NAME_MAP_ENTRY(BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP),
         NAME_MAP_ENTRY(BLE_GATTC_EVT_REL_DISC_RSP),
         NAME_MAP_ENTRY(BLE_GATTC_EVT_CHAR_DISC_RSP),
         NAME_MAP_ENTRY(BLE_GATTC_EVT_DESC_DISC_RSP),
         NAME_MAP_ENTRY(BLE_GATTC_EVT_ATTR_INFO_DISC_RSP),
         NAME_MAP_ENTRY(BLE_GATTC_EVT_CHAR_VAL_BY_UUID_READ_RSP),
         NAME_MAP_ENTRY(BLE_GATTC_EVT_READ_RSP),
         NAME_MAP_ENTRY(BLE_GATTC_EVT_CHAR_VALS_READ_RSP),
         NAME_MAP_ENTRY(BLE_GATTC_EVT_WRITE_RSP),
         NAME_MAP_ENTRY(BLE_GATTC_EVT_HVX)
#if NRF_SD_BLE_API >= 5
             ,
         NAME_MAP_ENTRY(BLE_GATTC_EVT_EXCHANGE_MTU_RSP),
         NAME_MAP_ENTRY(BLE_GATTC_EVT_TIMEOUT),
         NAME_MAP_ENTRY(BLE_GATTC_EVT_WRITE_CMD_TX_COMPLETE)
#endif // NRF_SD_BLE_API >= 5
        }};

    std::stringstream retval;

    try
    {
        retval << eventIds.at(eventId) << "(0x" << asHex(eventId) << ")";
    }
    catch (const std::out_of_range &)
    {
        retval << "UNKNOWN(0x" << asHex(eventId) << ")";
    }

    return retval.str();
}

std::string asText(const sd_rpc_log_severity_t &severity)
{
    switch (severity)
    {
        case SD_RPC_LOG_TRACE:
            return "TRACE";
        case SD_RPC_LOG_DEBUG:
            return "DEBUG";
        case SD_RPC_LOG_INFO:
            return "INFO";
        case SD_RPC_LOG_WARNING:
            return "WARNING";
        case SD_RPC_LOG_ERROR:
            return "ERROR";
        case SD_RPC_LOG_FATAL:
            return "FATAL";
        default:
            return "UNKNOWN";
    };
}

sd_rpc_log_severity_t parseLogSeverity(const std::string &level)
{
    if (level == "trace")
    {
        return SD_RPC_LOG_TRACE;
    }
    else if (level == "debug")
    {
        return SD_RPC_LOG_DEBUG;
    }
    else if (level == "info")
    {
        return SD_RPC_LOG_INFO;
    }
    else if (level == "warning")
    {
        return SD_RPC_LOG_WARNING;
    }
    else if (level == "error")
    {
        return SD_RPC_LOG_ERROR;
    }
    else if (level == "fatal")
    {
        return SD_RPC_LOG_FATAL;
    }

    std::stringstream ss;
    ss << "Not able to parse '" << level << "' to be sd_rpc_log_severity_t.";
    throw std::invalid_argument(ss.str());
}

spdlog::level::level_enum parseSpdLogLevel(const std::string &level)
{
    if (level == "trace")
    {
        return spdlog::level::level_enum::trace;
    }
    else if (level == "debug")
    {
        return spdlog::level::level_enum::debug;
    }
    else if (level == "info")
    {
        return spdlog::level::level_enum::info;
    }
    else if (level == "warning")
    {
        return spdlog::level::level_enum::warn;
    }
    else if (level == "error")
    {
        return spdlog::level::level_enum::err;
    }
    else if (level == "fatal")
    {
        return spdlog::level::level_enum::critical;
    }

    std::stringstream ss;
    ss << "Not able to parse '" << level << "' to be spdlog::level::level_enum.";
    throw std::invalid_argument(ss.str());
}
} // namespace testutil
