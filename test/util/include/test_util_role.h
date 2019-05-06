#pragma once

#include "ble.h"
#include <fmt/format.h>
#include <iostream>

namespace testutil {
enum class Role { Central = BLE_GAP_ROLE_CENTRAL, Peripheral = BLE_GAP_ROLE_PERIPH };
}; // namespace testutil

namespace fmt {
template <> struct formatter<testutil::Role>
{
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(const testutil::Role &role, FormatContext &ctx)
    {
        std::string role_text = "UNKNOWN";

        switch (role)
        {
            case testutil::Role::Central:
                role_text = "central";
                break;
            case testutil::Role::Peripheral:
                role_text = "periph ";
                break;
        };

        return format_to(ctx.begin(), "[{}]", role_text);
    }
};
}; // namespace fmt
