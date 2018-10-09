#ifndef TEST_UTIL_H__
#define TEST_UTIL_H__

#include "test_util_adapter_wrapper.h"
#include "test_util_conversion.h"
#include "test_util_role.h"

#include "ble.h"

#include <random>
#include <vector>
#include <algorithm>
#include <iterator>

namespace testutil
{
    static std::vector<std::shared_ptr<testutil::AdapterWrapper>> adapters;

    static void statusHandler(adapter_t *adapter, sd_rpc_app_status_t code, const char * message)
    {
        for (const auto& registeredAdapter : adapters)
        {
            if (registeredAdapter->unwrap()->internal == adapter->internal)
            {
                registeredAdapter->processStatus(code, message);
            }
        }
    }

    static void eventHandler(adapter_t *adapter, ble_evt_t *p_ble_evt)
    {
        for (const auto &registeredAdapter : adapters)
        {
            if (registeredAdapter->unwrap()->internal == adapter->internal)
            {
                registeredAdapter->processEvent(p_ble_evt);
            }
        }
    }

    static void logHandler(adapter_t *adapter, sd_rpc_log_severity_t severity, const char * log_message)
    {
        for (const auto &registeredAdapter : adapters)
        {
            if (registeredAdapter->unwrap()->internal == adapter->internal)
            {
                registeredAdapter->processLog(severity, log_message);
            }
        }
    }

    /**
     * @brief Function that find advertisement type data based on specified specified advertisement type
     *
     * @param[in]  advType Advertisment type to search for
     * @param[in]  advData Advertisement data to parse
     * @param[in,out] advTypeData Advertisement data found for provided advertisement type
     *
     * @return true if advertisement type was found, false if not.
     */
    static bool advReportParse(const uint8_t advType, const std::vector<uint8_t> &advData, std::vector<uint8_t> &advTypeData)
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

            if (fieldType == advType)
            {
                const auto advTypeDataLength = fieldLength - 1;
                const auto distance = std::distance(typeDataBegin, advData.end());

                if (distance < advTypeDataLength)
                {
                    return false;
                }

                const auto typeDataEnd = typeDataBegin + advTypeDataLength;
                advTypeData.assign(typeDataBegin, typeDataEnd);
                return true;
            }

            std::advance(typeDataBegin, fieldLength - 1);
        }

        return false;
    }

    /**
     * @brief Function that search for advertisement name in BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME or 
     * BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME
     *
     * @param[in] p_adv_report Pointer to advertisement report to search in
     * @param[in] name_to_find Advertisement name to search for
     *
     * @return true if name is found, false if not
     */
    static bool findAdvName(const ble_gap_evt_adv_report_t *p_adv_report, const std::string &name_to_find)
    {
        std::vector<uint8_t> advData;

        uint8_t *data;
        uint16_t data_len;

#if NRF_SD_BLE_API >= 6
        data = (uint8_t*)p_adv_report->data.p_data;
        data_len = p_adv_report->data.len;
#else
        data = (uint8_t*)p_adv_report->data;
        data_len = p_adv_report->dlen;
#endif
        advData.assign(data, data + data_len);

        std::vector<uint8_t> advTypeData;
        std::vector<uint8_t> nameToFind;
        std::copy(name_to_find.begin(), name_to_find.end(), std::back_inserter(nameToFind));

        auto found = [&](const uint8_t advType)
        {
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

    /**
     * @brief Function that append name to advertise to advertisement type BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME
     *
     * @param[in,out] advertisingData std::vector to append advertisement data to
     * @param[in] name Name to append to advertisingData
     */
    static void appendAdvertisingName(std::vector<uint8_t> &advertisingData, const std::string &name)
    {
        advertisingData.push_back((uint8_t)(name.length() + 1)); // Device name + advertisement type
        advertisingData.push_back(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME);
        std::copy(name.begin(), name.end(), std::back_inserter(advertisingData));
    }

    /*
     * @brief Function that fills a std::vector<uint8_t> with random values
     *
     * @param[in,out] data vector to populate with random values
     * @param[in] size number of random values to fill the vector with
     */
    static void appendRandomData(std::vector<uint8_t> &data, const size_t size)
    {
        data.resize(size);
        std::generate(data.begin(), data.end(), std::rand);
    }
}

#endif // TEST_UTIL_H__
