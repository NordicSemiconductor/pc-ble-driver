#ifndef TEST_UTIL_H__
#define TEST_UTIL_H__

namespace testutil
{
    std::string convertToString(const std::vector<uint8_t> &data) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');

        for (uint8_t const& value : data)
        {
            ss << std::setw(2) << std::hex << static_cast<int>(value) << ' ';
        }

        return ss.str();
    }
}

#endif // TEST_UTIL_H__