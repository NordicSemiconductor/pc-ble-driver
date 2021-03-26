/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 *   3. Neither the name of Nordic Semiconductor ASA nor the names of other
 *   contributors to this software may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *   4. This software must only be used in or with a processor manufactured by Nordic
 *   Semiconductor ASA, or in or with a processor manufactured by a third party that
 *   is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 *   5. Any software provided in binary or object form under this license must not be
 *   reverse engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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
