/** @file
 *
 * @brief Helper for special string conversions.
 *
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <optional>
#include <string>

/**@brief String helper class.
 *
 * This is a static helper class for performing special string operations.
 */
class StringHelper
{
  public:
    static auto toCString(const std::optional<std::string> &cpp_str) noexcept -> const char *
    {
        if (cpp_str.has_value())
        {
            try
            {
                return cpp_str->c_str();
            }
            catch (...)
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }

    static auto toOptional(const char *c_str) noexcept -> std::optional<std::string>
    {
        if (c_str != nullptr)
        {
            try
            {
                return std::make_optional<std::string>(std::string(c_str));
            }
            catch (...)
            {
                return std::optional<std::string>{};
            }
        }

        return std::optional<std::string>{};
    }

    static auto subStringAfterWord(const std::string &source, const std::string &word,
                                   size_t substr_length, std::string &target) -> bool
    {
        auto word_i = source.find(word, 0);

        if (word_i == std::string::npos)
        {
            return false;
        }

        target = source.substr(word_i + word.length(), substr_length);

        return true;
    }

    static void toUpper(std::string &thestring)
    {
        std::transform(thestring.begin(), thestring.end(), thestring.begin(),
                       [](unsigned char c) -> unsigned char { return std::toupper(c); });
    }
};
