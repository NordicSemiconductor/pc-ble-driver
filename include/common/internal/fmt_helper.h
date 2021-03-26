#pragma once

#include <fmt/format.h>

#include "h5_transport.h"
#include "uart_settings.h"
#include "uart_settings_boost.h"
#include "uart_transport.h"

namespace fmt {
template <> struct formatter<h5_state>
{
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(const h5_state state, FormatContext &ctx)
    {
        switch (state)
        {
            case h5_state::STATE_START:
                format_to(ctx.out(), "STATE_START");
                break;
            case h5_state::STATE_RESET:
                format_to(ctx.out(), "STATE_RESET");
                break;
            case h5_state::STATE_UNINITIALIZED:
                format_to(ctx.out(), "STATE_UNINITIALIZED");
                break;
            case h5_state::STATE_INITIALIZED:
                format_to(ctx.out(), "STATE_INITIALIZED");
                break;
            case h5_state::STATE_ACTIVE:
                format_to(ctx.out(), "STATE_ACTIVE");
                break;
            case h5_state::STATE_FAILED:
                format_to(ctx.out(), "STATE_FAILED");
                break;
            case h5_state::STATE_CLOSED:
                format_to(ctx.out(), "STATE_CLOSED");
                break;
            case h5_state::STATE_NO_RESPONSE:
                format_to(ctx.out(), "STATE_NO_RESPONSE");
                break;
            case h5_state::STATE_UNKNOWN:
                format_to(ctx.out(), "STATE_UNKNOWN");
                break;
            default:
                format_to(ctx.out(), "UNKNOWN[{:#04x}]", static_cast<uint32_t>(state));
        }
    }
};

template <> struct formatter<std::vector<uint8_t>>
{
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::vector<uint8_t> &payload, FormatContext &ctx)
    {
        format_to(ctx.out(), "{:#04x}", join(payload, ", "));
    }
};

template <> struct formatter<h5_pkt_type>
{
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(const h5_pkt_type &pkt_type, FormatContext &ctx)
    {
        switch (pkt_type)
        {
            case h5_pkt_type::ACK_PACKET:
                format_to(ctx.out(), "ACK");
                break;
            case h5_pkt_type::HCI_COMMAND_PACKET:
                format_to(ctx.out(), "HCI_COMMAND");
                break;
            case h5_pkt_type::ACL_DATA_PACKET:
                format_to(ctx.out(), "ACL_DATA");
                break;
            case h5_pkt_type::SYNC_DATA_PACKET:
                format_to(ctx.out(), "SYNC_DATA");
                break;
            case h5_pkt_type::HCI_EVENT_PACKET:
                format_to(ctx.out(), "HCI_EVENT");
                break;
            case h5_pkt_type::RESET_PACKET:
                format_to(ctx.out(), "RESET");
                break;
            case h5_pkt_type::VENDOR_SPECIFIC_PACKET:
                format_to(ctx.out(), "VENDOR_SPECIFIC");
                break;
            case h5_pkt_type::LINK_CONTROL_PACKET:
                format_to(ctx.out(), "LINK_CONTROL");
                break;
            default:
                format_to(ctx.out(), "UNKNOWN[{:#04x}]", static_cast<uint32_t>(pkt_type));
        }
    }
};

template <> struct formatter<UartParity>
{
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(const UartParity &parity, FormatContext &ctx)
    {
        switch (parity)
        {
            case UartParityNone:
                format_to(ctx.out(), "none");
                break;
            case UartParityOdd:
                format_to(ctx.out(), "odd");
                break;
            case UartParityEven:
                format_to(ctx.out(), "even");
                break;
            default:
                format_to(ctx.out(), "unknown");
        }
    }
};

template <> struct formatter<UartFlowControl>
{
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const UartFlowControl &flowControl, FormatContext &ctx)
    {
        switch (flowControl)
        {
            case UartFlowControlNone:
                format_to(ctx.out(), "none");
                break;
            case UartFlowControlSoftware:
                format_to(ctx.out(), "software");
                break;
            case UartFlowControlHardware:
                format_to(ctx.out(), "hardware");
                break;
            default:
                format_to(ctx.out(), "unknown");
        }
    }
};

template <> struct formatter<asio::serial_port::parity>
{
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const asio::serial_port::parity &parity, FormatContext &ctx)
    {
        switch (parity)
        {
            case asio::serial_port::parity::none:
                format_to(ctx.out(), "none");
                break;
            case asio::serial_port::parity::odd:
                format_to(ctx.out(), "odd");
                break;
            case asio::serial_port::parity::even:
                format_to(ctx.out(), "even");
                break;
            default:
                format_to(ctx.out(), "unknown");
        }
    }
};

template <> struct formatter<asio::serial_port::flow_control>
{
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    constexpr auto parse(format_parse_context &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const asio::serial_port::flow_control &flowControl, FormatContext &ctx)
    {
        switch (flowControl)
        {
            case asio::serial_port::flow_control::none:
                format_to(ctx.out(), "none");
                break;
            case asio::serial_port::flow_control::software:
                format_to(ctx.out(), "software");
                break;
            case asio::serial_port::flow_control::hardware:
                format_to(ctx.out(), "hardware");
                break;
            default:
                format_to(ctx.out(), "unknown");
        }
    }
};
} // namespace fmt