#include "test_util.h"

#include "ble.h"

#include <algorithm>
#include <cstring>
#include <iterator>

#include <cctype>
#include <random>
#include <vector>

namespace testutil {

bool advReportParse(const uint8_t advType, const std::vector<uint8_t> &advData,
                    std::vector<uint8_t> &advTypeData)
{
    auto typeDataBegin = advData.begin();
    while (typeDataBegin != advData.end())
    {
        auto fieldLength = *typeDataBegin++;
        if (typeDataBegin == advData.end())
        {
            return false;
        }

        auto fieldType = *typeDataBegin++;
        if (typeDataBegin == advData.end())
        {
            return false;
        }

        const auto advTypeDataLength = fieldLength - 1;
        const auto distance          = std::distance(typeDataBegin, advData.end());

        if (distance < advTypeDataLength)
        {
            return false;
        }

        if (fieldType == advType)
        {
            const auto typeDataEnd = typeDataBegin + advTypeDataLength;
            advTypeData.assign(typeDataBegin, typeDataEnd);
            return true;
        }

        std::advance(typeDataBegin, fieldLength - 1);
    }

    return false;
}

bool findManufacturerSpecificData(const ble_gap_evt_adv_report_t &p_adv_report,
                                  std::vector<uint8_t> &manufacturer_specific_data)
{
    std::vector<uint8_t> advData;

    uint8_t *data;
    uint16_t data_len;

#if NRF_SD_BLE_API >= 6
    data     = (uint8_t *)p_adv_report.data.p_data;
    data_len = p_adv_report.data.len;
#else
    data     = (uint8_t *)p_adv_report.data;
    data_len = p_adv_report.dlen;
#endif
    advData.assign(data, data + data_len);

    std::vector<uint8_t> manufacturerSpecific;
    return advReportParse(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, advData,
                          manufacturer_specific_data);
}

bool findAdvName(const ble_gap_evt_adv_report_t p_adv_report, const std::string &name_to_find)
{
    std::vector<uint8_t> advData;

    uint8_t *data;
    uint16_t data_len;

#if NRF_SD_BLE_API >= 6
    data     = (uint8_t *)p_adv_report.data.p_data;
    data_len = p_adv_report.data.len;
#else
    data     = (uint8_t *)p_adv_report.data;
    data_len = p_adv_report.dlen;
#endif
    advData.assign(data, data + data_len);

    std::vector<uint8_t> advTypeData;
    std::vector<uint8_t> nameToFind;
    std::copy(name_to_find.begin(), name_to_find.end(), std::back_inserter(nameToFind));

    auto found = [&](const uint8_t advType) {
        if (advReportParse(advType, advData, advTypeData))
        {
            if (nameToFind.size() > advTypeData.size())
            {
                return false;
            }

            if (std::equal(nameToFind.begin(), nameToFind.end(), advTypeData.begin()))
            {
                return true;
            }
        }

        return false;
    };

    return found(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME) || found(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME);
}

void assertAdvertisingPacketSizeIsValid(const std::vector<uint8_t> existingPacket,
                                        const size_t additionalBytes, const bool extended,
                                        const bool connectable)
{
    auto valid            = true;
    const auto packetSize = existingPacket.size() + additionalBytes;

    if (packetSize > testutil::ADV_DATA_BUFFER_SIZE)
    {
        if (extended)
        {
#if NRF_SD_BLE_API == 6
            valid = packetSize >
                    static_cast<size_t>(
                        connectable ? BLE_GAP_ADV_SET_DATA_SIZE_EXTENDED_CONNECTABLE_MAX_SUPPORTED
                                    : BLE_GAP_ADV_SET_DATA_SIZE_EXTENDED_MAX_SUPPORTED);
#else
            throw std::invalid_argument(
                "Only SoftDevice API version 6 or newer support extended advertising.");
#endif
        }
        else
        {
#if NRF_SD_BLE_API == 6
            valid = packetSize > BLE_GAP_ADV_SET_DATA_SIZE_MAX;
#else
            valid = packetSize > BLE_GAP_ADV_MAX_SIZE;
#endif
        }
    }

    if (!valid)
    {
        throw std::range_error("Packet size is larger than allowed for this type of advertising.");
    }
}

void appendAdvertisingName(std::vector<uint8_t> &advertisingData, const std::string &name,
                           const bool extended)
{
    assertAdvertisingPacketSizeIsValid(advertisingData, 2 /* size + type */ + name.length(),
                                       extended);

    advertisingData.push_back(1 /* type */ + static_cast<uint8_t>(name.length()));
    advertisingData.push_back(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME);
    std::copy(name.begin(), name.end(), std::back_inserter(advertisingData));
}

void appendAdvertisingFlags(std::vector<uint8_t> &advertisingData, const uint8_t flags,
                            const bool extended)
{
    assertAdvertisingPacketSizeIsValid(advertisingData, 3 /* size + type + flags */, extended);

    advertisingData.push_back(2); // Flags field size + flags data size
    advertisingData.push_back(BLE_GAP_AD_TYPE_FLAGS);
    advertisingData.push_back(flags);
}

void appendManufacturerSpecificData(std::vector<uint8_t> &advertisingData,
                                    const std::vector<uint8_t> &manufacturerSpecificData,
                                    const bool extended)
{
    assertAdvertisingPacketSizeIsValid(
        advertisingData, 2 /* size + type */ + manufacturerSpecificData.size(), extended);

    advertisingData.push_back(1 /* type */ + static_cast<uint8_t>(manufacturerSpecificData.size()));
    advertisingData.push_back(BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA);
    std::copy(manufacturerSpecificData.begin(), manufacturerSpecificData.end(),
              std::back_inserter(advertisingData));
}

void appendRandomData(std::vector<uint8_t> &data, const size_t size)
{
    data.resize(size);
    std::generate(data.begin(), data.end(), std::rand);
}

void appendRandomAlphaNumeric(std::vector<uint8_t> &data, const size_t size)
{
    data.resize(size);
    std::generate(data.begin(), data.end(), [] {
        uint8_t c;
        while (!std::isalnum(c = static_cast<uint8_t>(std::rand())))
            ;
        return c;
    });
}

std::string createRandomAdvertisingName(const std::string &prefix, const uint8_t length)
{
    std::vector<uint8_t> data;
    data.reserve(length);

    if (prefix.length() > length)
    {
        return std::string(prefix.begin(), prefix.begin() + length);
    }

    std::copy(prefix.begin(), prefix.end(), std::back_inserter(data));
    const auto randomLength = length - prefix.length();
    std::generate_n(std::back_inserter(data), randomLength, [] {
        uint8_t c;
        while (!std::isalnum(c = static_cast<uint8_t>(std::rand())))
            ;
        return c;
    });

    return std::string(data.begin(), data.end());
}

void createRandomAdvertisingData(std::vector<uint8_t> &advertisingData,
                                 std::string &advertisingName, std::vector<uint8_t> &randomData,
                                 const size_t advertisingNameLength,
                                 const size_t manufacturerDataLength, const bool extended)
{
    randomData.clear();
    randomData.resize(manufacturerDataLength);

    std::vector<uint8_t> peripheralAdvNameBuffer(advertisingNameLength);
    appendRandomAlphaNumeric(peripheralAdvNameBuffer, advertisingNameLength);
    const std::string peripheralAdvName(peripheralAdvNameBuffer.begin(),
                                        peripheralAdvNameBuffer.end());
    testutil::appendAdvertisingName(advertisingData, peripheralAdvName);
    advertisingName = peripheralAdvName;

    testutil::appendRandomData(randomData, manufacturerDataLength);
    testutil::appendManufacturerSpecificData(advertisingData, randomData, extended);
}

bool operator==(const ble_gap_addr_t &lhs, const ble_gap_addr_t &rhs)
{
    const auto addressIsSame = std::memcmp(lhs.addr, rhs.addr, BLE_GAP_ADDR_LEN) == 0;

    if (addressIsSame && (lhs.addr_type == rhs.addr_type))
    {
#if NRF_SD_BLE_API == 6
        if (lhs.addr_id_peer != rhs.addr_id_peer)
        {
            return false;
        }
#endif // NRF_SD_BLE_API

        return true;
    }
    else
    {
        return false;
    }
}

bool operator!=(const ble_gap_addr_t &lhs, const ble_gap_addr_t &rhs)
{
    return !(lhs == rhs);
}

#if NRF_SD_BLE_API == 6
bool operator==(const ble_gap_phys_t &lhs, const ble_gap_phys_t &rhs)
{
    return ((lhs.tx_phys == rhs.tx_phys) && (lhs.rx_phys == rhs.rx_phys));
}

bool operator!=(const ble_gap_phys_t &lhs, const ble_gap_phys_t &rhs)
{
    return !(lhs == rhs);
}
#endif // NRF_SD_BLE_API == 6

} // namespace testutil
