#pragma once

#include <ble.h>
#include <ble_hci.h>
#include <sd_rpc_types.h>

#include <spdlog/logger.h>
#include <string>
#include <vector>

namespace testutil {
/**
 * @brief Function that convert data to string representation
 * @param[in] data Data to represent as a string
 * @return string representation of data
 */
std::string asHex(const std::vector<uint8_t> &data);
std::string asHex(const uint16_t &data);

/**
 * @brief Function that convert a Bluetooth Low Energy address to string
 *
 * @param[in] address Bluetooth Lower Energy address
 * @return string representation of provided address
 */
std::string asText(const ble_gap_addr_t &address);

/**
 * @brief Function that convert an error code to string representation
 *
 * param[in] error code
 *
 * @return textual representation of the error
 */
std::string errorToString(const uint32_t error_code);
std::string ioCapsToString(const uint8_t code);
std::string roleToString(const uint8_t role);
std::string gattAuthErrorSrcToString(const uint8_t errorSrc);
std::string gattAuthStatusToString(const uint8_t authStatus);
std::string hciStatusCodeToString(const uint8_t hciStatusCode);
std::string eventIdAsText(const uint32_t eventId);

std::string asText(const ble_gap_evt_passkey_display_t &passkeyDisplay);
std::string asText(const ble_gap_enc_info_t &encryptionInfo);
std::string asText(const ble_gap_conn_params_t &connectionParams);
std::string asText(const ble_gap_evt_connected_t &connected);
std::string asText(const ble_gap_evt_disconnected_t &disconnected);
std::string asText(const ble_gap_evt_scan_req_report_t &scanReqReport);
std::string asText(const ble_gap_sec_kdist_t &keyDist);
std::string asText(const ble_gap_evt_sec_request_t &securityRequest);
std::string asText(const ble_gap_evt_sec_params_request_t &securityParamsRequest);
std::string asText(const ble_gap_evt_auth_key_request_t &authKeyRequest);
std::string asText(const ble_gap_conn_sec_mode_t &connSecMode);
std::string asText(const ble_gap_conn_sec_t &connSec);
std::string asText(const ble_gap_evt_rssi_changed_t &rssiChanged);
std::string asText(const ble_gap_lesc_p256_pk_t &lsecP256Pk);
std::string asText(const ble_gap_evt_key_pressed_t &keyPressed);
std::string asText(const std::vector<uint8_t> &data);
std::string asText(const ble_gap_evt_timeout_t &timeout);

#if NRF_SD_BLE_API == 6
std::string asText(const ble_gap_adv_report_type_t &reportType);
std::string asText(const ble_gap_aux_pointer_t &auxPointer);
std::string asText(const ble_gap_phys_t &phys);
std::string asText(const ble_gap_evt_phy_update_request_t &phyUpdateRequest);
std::string asText(const ble_gap_evt_phy_update_t &phyUpdate);
std::string asText(const ble_gap_data_length_params_t &dataLengthParams);
std::string asText(const ble_gap_evt_data_length_update_request_t &dataLengthUpdateRequest);
std::string asText(const ble_gap_evt_data_length_update_t &dataLengthUpdate);
std::string asText(const ble_gap_evt_qos_channel_survey_report_t &qosChannelSurveyReport);
std::string asText(const ble_data_t &data);
std::string asText(const ble_gap_adv_data_t &advData);
std::string asText(const ble_gap_evt_adv_set_terminated_t &advSetTerminated);
#endif // NRF_SD_BLE_API == 6

std::string asText(const ble_gap_evt_adv_report_t &advReport);
std::string asText(const ble_gap_evt_lesc_dhkey_request_t &dhkeyRequest);
std::string asText(const ble_gap_master_id_t &masterId);
std::string asText(const ble_gap_evt_sec_info_request_t &secInfoRequest);
std::string asText(const ble_gap_evt_conn_sec_update_t &connSecUpdate);
std::string asText(const ble_gap_sec_levels_t &secLevels);
std::string asText(const ble_gap_evt_auth_status_t &authStatus);

/**
 * @brief Function that convert a GATT status code to string representation
 *
 * param[in] code code
 *
 * @return textual representation of the code
 */
std::string gattStatusToString(const uint16_t code);

// Operator overloading for common types used
std::string asText(const sd_rpc_app_status_t &status);
std::string asText(const uint16_t &value);
std::string asText(const uint8_t &value);
std::string asText(const ble_uuid_t &uuid);
std::string asText(const ble_gattc_char_t &gattc_char);
std::string asText(const ble_gattc_handle_range_t &gattc_handle_range);
std::string asText(const ble_gattc_desc_t &gattc_desc);
std::string asText(const sd_rpc_log_severity_t &severity);

sd_rpc_log_severity_t parseLogSeverity(const std::string &level);
spdlog::level::level_enum parseSpdLogLevel(const std::string &level);

} // namespace testutil
