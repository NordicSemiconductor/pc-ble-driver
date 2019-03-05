#pragma once

#include "test_util_adapter_wrapper.h"
#include "test_util_conversion.h"
#include "test_util_role.h"

#include "ble.h"

#include <cstring>
#include <iterator>
#include <random>
#include <vector>

namespace testutil {

/**
 * @brief Function that find advertising type data based on specified specified advertising type
 *
 * @param[in]  advType Advertising type to search for
 * @param[in]  advData Advertising data to parse
 * @param[in,out] advTypeData Advertising data found for provided advertising type
 *
 * @return true if advertising type was found, false if not.
 */
bool advReportParse(const uint8_t advType, const std::vector<uint8_t> &advData,
                    std::vector<uint8_t> &advTypeData);

bool findManufacturerSpecificData(const ble_gap_evt_adv_report_t &p_adv_report,
                                  std::vector<uint8_t> &manufacturer_specific_data);

/**
 * @brief Function that search for advertising name in BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME or
 * BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME
 *
 * @param[in] p_adv_report Pointer to advertising report to search in
 * @param[in] name_to_find Advertising name to search for
 *
 * @return true if name is found, false if not
 */
bool findAdvName(const ble_gap_evt_adv_report_t p_adv_report, const std::string &name_to_find);

void assertAdvertisingPacketSizeIsValid(const std::vector<uint8_t> existingPacket,
                                        const size_t additionalBytes, const bool extended = false,
                                        const bool connectable = false);

/**
 * @brief Function that append name to advertise to advertising type
 * BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME
 *
 * @param[in,out] advertisingData std::vector to append advertising data to
 * @param[in] name Name to append to advertisingData
 * @param[in] extended specifies if packet type is extended or not
 */
void appendAdvertisingName(std::vector<uint8_t> &advertisingData, const std::string &name,
                           const bool extended = false);

/**
 * @brief Function that append flags to advertising type
 *
 * @param[in,out] advertisingData std::vector to append advertising flags to
 * @param[in] flags Flags to append to advertisingData
 * @param[in] extended specifies if packet type is extended or not
 */
void appendAdvertisingFlags(std::vector<uint8_t> &advertisingData, const uint8_t flags,
                            const bool extended = false);

/**
 * @brief Function that append manufacturer specifid data to advertising
 *
 * @param[in,out] advertisingData std::vector to append manufacturer specific data to
 * @param[in] manufacturerSpecificData Manufacturer specific data to append to advertisingData
 * @param[in] extended specifies if packet type is extended or not
 */
void appendManufacturerSpecificData(std::vector<uint8_t> &advertisingData,
                                    const std::vector<uint8_t> &manufacturerSpecificData,
                                    const bool extended = false);

/**
 * @brief Function that fills a std::vector<uint8_t> with random values
 *
 * @param[in,out] data vector to populate with random values
 * @param[in] size number of random values to fill the vector with
 */
void appendRandomData(std::vector<uint8_t> &data, const size_t size);

/**
 * @brief Function that fills a std::vector<uint8_t> with random ASCII values
 *
 * @param[in,out] data vector to populate with random ASCII values
 * @param[in] size number of random values to fill the vector with
 */
void appendRandomAlphaNumeric(std::vector<uint8_t> &data, const size_t size);

/**
 * @brief Function that create a random advertising name
 *
 * @param[in] prefix A prefix added to the advertising name
 * @param[in] length Total length of advertising name (including prefix)
 */
std::string createRandomAdvertisingName(const std::string &prefix = "periph",
                                        const uint8_t length      = 10);

/**
 * @brief Function that append random advertising data
 *
 * @param[out] advertisingData vector to append advertising data to
 * @param[out] advertisingName vector to populate with randomly created advertising name
 * @param[out] manufacturerData vector to populate with randomly created manufacturer data
 *
 */
void createRandomAdvertisingData(
    std::vector<uint8_t> &advertisingData, std::string &advertisingName,
    std::vector<uint8_t> &randomData, const size_t advertisingNameLength = 20,
    const size_t manufacturerDataLength = testutil::ADV_DATA_BUFFER_SIZE -
                                          20 /* advertisingNameLength */ -
                                          2 /* length and AD type */ - 2 /* length and AD type */,
    const bool extended = false);

bool operator==(const ble_gap_addr_t &lhs, const ble_gap_addr_t &rhs);

bool operator!=(const ble_gap_addr_t &lhs, const ble_gap_addr_t &rhs);

#if NRF_SD_BLE_API == 6
bool operator==(const ble_gap_phys_t &lhs, const ble_gap_phys_t &rhs);
bool operator!=(const ble_gap_phys_t &lhs, const ble_gap_phys_t &rhs);
#endif // NRF_SD_BLE_API == 6

} // namespace testutil
