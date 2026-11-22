#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Compucolor::Impl::App
{
    enum class TraceFilterKind {
        Cpu,
        Instr,
        Port,
        Tms5501,
        Smc5027
    };

    enum class TraceFilterDirection {
        Any,
        Read,
        Write
    };

    struct TraceFilterAddressRange {
        uint16_t from = 0;
        uint16_t to = 0;

        [[nodiscard]] bool Contains(const uint16_t value) const noexcept
        {
            return value >= from && value <= to;
        }
    };

    struct TraceFilter {
        TraceFilterKind kind = TraceFilterKind::Cpu;

        bool hasPc = false;
        TraceFilterAddressRange pc;

        bool hasPort = false;
        TraceFilterAddressRange port;

        bool hasDirection = false;
        TraceFilterDirection direction = TraceFilterDirection::Any;

        // Stored lowercase so text matching is case-insensitive.
        std::string text;
    };

    // Parsed CLI trace filters. Empty means "no filter supplied", which
    // preserves the original fully-unfiltered JSONL output.
    class TraceFilterConfig {
        public:
            TraceFilterConfig() = default;

            static TraceFilterConfig Parse(const std::vector<std::string>& specs);

            [[nodiscard]] bool Empty() const noexcept;
            [[nodiscard]] bool WantsText(const std::string& kind) const;
            [[nodiscard]] bool Allows(
                const std::string& kind,
                uint16_t pc = 0,
                uint16_t port = 0,
                bool isWrite = false,
                const std::string& text = {}
            ) const;

        private:
            std::vector<TraceFilter> _filters;
    };
}
