#ifndef TEST_UTIL_H__
#define TEST_UTIL_H__

namespace testutil
{
    static std::string convertToString(const std::vector<uint8_t> &data) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');

        for (uint8_t const& value : data)
        {
            ss << std::setw(2) << std::hex << static_cast<int>(value) << ' ';
        }

        return ss.str();
    }

    static std::string ble_address_to_string_convert(ble_gap_addr_t address)
    {
        const int address_length = 6;
        std::stringstream retval;

        for (int i = sizeof(address.addr) - 1; i >= 0; --i)
        {
            retval << std::hex << static_cast<unsigned int>(address.addr[i]);
        }
        
        return retval.str();
    }
}

#endif // TEST_UTIL_H__