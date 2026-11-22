#include <Compucolor/App/TraceFilterConfig.h>

#include <algorithm>
#include <cctype>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace
{
    using Compucolor::Impl::App::TraceFilter;
    using Compucolor::Impl::App::TraceFilterAddressRange;
    using Compucolor::Impl::App::TraceFilterConfig;
    using Compucolor::Impl::App::TraceFilterDirection;
    using Compucolor::Impl::App::TraceFilterKind;

    [[noreturn]] void Invalid(const std::string& spec, const std::string& message)
    {
        throw std::runtime_error("Invalid trace filter '" + spec + "': " + message);
    }

    std::string Lower(std::string value)
    {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](const unsigned char character) {
                return static_cast<char>(std::tolower(character));
            }
        );

        return value;
    }

    std::string_view Trim(const std::string_view value)
    {
        const auto first = value.find_first_not_of(" \t");

        if (first == std::string_view::npos)
        {
            return {};
        }

        const auto last = value.find_last_not_of(" \t");
        return value.substr(first, last - first + 1);
    }

    TraceFilterKind ParseKind(const std::string& spec, const std::string_view name)
    {
        const std::string lower(name);

        if (lower == "cpu")
        {
            return TraceFilterKind::Cpu;
        }

        if (lower == "instr")
        {
            return TraceFilterKind::Instr;
        }

        if (lower == "port")
        {
            return TraceFilterKind::Port;
        }

        if (lower == "tms5501")
        {
            return TraceFilterKind::Tms5501;
        }

        if (lower == "smc5027")
        {
            return TraceFilterKind::Smc5027;
        }

        Invalid(spec, "unknown kind '" + std::string(name) + "'");
    }

    uint16_t ParseHex(
        const std::string& spec,
        const std::string& key,
        const std::string_view value,
        const uint16_t maximum,
        const std::string& label
    )
    {
        std::string_view digits = value;

        if (digits.size() >= 2 && digits[0] == '0' &&
            (digits[1] == 'x' || digits[1] == 'X'))
        {
            digits.remove_prefix(2);
        }

        if (digits.empty() || digits.size() > 4)
        {
            Invalid(spec, key + " must be a hex " + label);
        }

        uint32_t parsed = 0;

        for (const char character : digits)
        {
            if (!std::isxdigit(static_cast<unsigned char>(character)))
            {
                Invalid(spec, key + " must be a hex " + label);
            }

            const char upper = static_cast<char>(
                std::toupper(static_cast<unsigned char>(character))
            );
            const uint32_t digit = upper >= 'A'
                ? static_cast<uint32_t>(upper - 'A' + 10)
                : static_cast<uint32_t>(upper - '0');

            parsed = (parsed << 4) | digit;
        }

        if (parsed > maximum)
        {
            Invalid(spec, key + " " + label + " is out of range");
        }

        return static_cast<uint16_t>(parsed);
    }

    TraceFilterAddressRange ParseRange(
        const std::string& spec,
        const std::string& key,
        const std::string_view value,
        const uint16_t maximum,
        const std::string& label
    )
    {
        const auto dash = value.find('-');

        if (dash == std::string_view::npos)
        {
            const uint16_t address = ParseHex(spec, key, value, maximum, label);
            return {address, address};
        }

        if (value.find('-', dash + 1) != std::string_view::npos)
        {
            Invalid(spec, key + " must be a single value or an inclusive range");
        }

        const uint16_t from = ParseHex(
            spec,
            key,
            Trim(value.substr(0, dash)),
            maximum,
            label
        );
        const uint16_t to = ParseHex(
            spec,
            key,
            Trim(value.substr(dash + 1)),
            maximum,
            label
        );

        if (from > to)
        {
            Invalid(spec, key + " range starts after it ends");
        }

        return {from, to};
    }

    bool KeyAllowedForKind(const TraceFilterKind kind, const std::string& key)
    {
        switch (kind)
        {
            case TraceFilterKind::Cpu:
                return key == "pc";
            case TraceFilterKind::Instr:
                return key == "pc" || key == "text";
            case TraceFilterKind::Port:
                return key == "pc" || key == "port" || key == "dir";
            case TraceFilterKind::Tms5501:
            case TraceFilterKind::Smc5027:
                return false;
        }

        return false;
    }

    void ApplyKey(TraceFilter& filter, const std::string& spec, const std::string_view segment)
    {
        const auto equals = segment.find('=');

        if (equals == std::string_view::npos || equals == 0)
        {
            Invalid(spec, "expected key=value in '" + std::string(segment) + "'");
        }

        const std::string key(Trim(segment.substr(0, equals)));
        const std::string_view value = Trim(segment.substr(equals + 1));

        if (key.empty())
        {
            Invalid(spec, "key cannot be empty");
        }

        if (value.empty())
        {
            Invalid(spec, key + " cannot be empty");
        }

        if (!KeyAllowedForKind(filter.kind, key))
        {
            Invalid(spec, "key '" + key + "' is not valid for this kind");
        }

        if (key == "pc")
        {
            if (filter.hasPc)
            {
                Invalid(spec, "duplicate key 'pc'");
            }

            filter.hasPc = true;
            filter.pc = ParseRange(spec, key, value, 0xFFFF, "address");
            return;
        }

        if (key == "port")
        {
            if (filter.hasPort)
            {
                Invalid(spec, "duplicate key 'port'");
            }

            filter.hasPort = true;
            filter.port = ParseRange(spec, key, value, 0xFF, "port");
            return;
        }

        if (key == "dir")
        {
            if (filter.hasDirection)
            {
                Invalid(spec, "duplicate key 'dir'");
            }

            const std::string direction = Lower(std::string(value));

            if (direction == "any")
            {
                filter.direction = TraceFilterDirection::Any;
            }
            else if (direction == "read")
            {
                filter.direction = TraceFilterDirection::Read;
            }
            else if (direction == "write")
            {
                filter.direction = TraceFilterDirection::Write;
            }
            else
            {
                Invalid(spec, "dir must be read, write, or any");
            }

            filter.hasDirection = true;
            return;
        }

        if (key == "text")
        {
            if (!filter.text.empty())
            {
                Invalid(spec, "duplicate key 'text'");
            }

            filter.text = Lower(std::string(value));
        }
    }

    std::optional<TraceFilterKind> KindFromName(const std::string& name)
    {
        if (name == "cpu")
        {
            return TraceFilterKind::Cpu;
        }

        if (name == "instr")
        {
            return TraceFilterKind::Instr;
        }

        if (name == "port")
        {
            return TraceFilterKind::Port;
        }

        if (name == "tms5501")
        {
            return TraceFilterKind::Tms5501;
        }

        if (name == "smc5027")
        {
            return TraceFilterKind::Smc5027;
        }

        return std::nullopt;
    }
}

TraceFilterConfig TraceFilterConfig::Parse(const std::vector<std::string>& specs)
{
    TraceFilterConfig config;

    for (const std::string& spec : specs)
    {
        if (spec.empty())
        {
            Invalid(spec, "filter is empty");
        }

        const auto colon = spec.find(':');
        const std::string_view kindName = colon == std::string::npos
            ? std::string_view(spec)
            : std::string_view(spec).substr(0, colon);
        const std::string_view keys = colon == std::string::npos
            ? std::string_view()
            : std::string_view(spec).substr(colon + 1);

        TraceFilter filter;
        filter.kind = ParseKind(spec, kindName);

        if (colon != std::string::npos && keys.empty())
        {
            Invalid(spec, "missing keys after ':'");
        }

        if (colon != std::string::npos)
        {
            std::size_t start = 0;

            while (start <= keys.size())
            {
                const auto comma = keys.find(',', start);
                const auto length = comma == std::string_view::npos
                    ? std::string_view::npos
                    : comma - start;
                const std::string_view segment = keys.substr(start, length);

                if (segment.empty())
                {
                    Invalid(spec, "empty key segment");
                }

                ApplyKey(filter, spec, segment);

                if (comma == std::string_view::npos)
                {
                    break;
                }

                start = comma + 1;
            }
        }

        config._filters.push_back(std::move(filter));
    }

    return config;
}

bool TraceFilterConfig::Empty() const noexcept
{
    return _filters.empty();
}

bool TraceFilterConfig::WantsText(const std::string& kind) const
{
    const auto wanted = KindFromName(kind);

    if (!wanted.has_value())
    {
        return false;
    }

    for (const TraceFilter& filter : _filters)
    {
        if (filter.kind == wanted.value() && !filter.text.empty())
        {
            return true;
        }
    }

    return false;
}

bool TraceFilterConfig::Allows(
    const std::string& kind,
    const uint16_t pc,
    const uint16_t port,
    const bool isWrite,
    const std::string& text
) const
{
    if (_filters.empty())
    {
        return true;
    }

    const auto wanted = KindFromName(kind);

    if (!wanted.has_value())
    {
        return false;
    }

    const std::string loweredText = WantsText(kind) ? Lower(text) : std::string();

    for (const TraceFilter& filter : _filters)
    {
        if (filter.kind != wanted.value())
        {
            continue;
        }

        if (filter.hasPc && !filter.pc.Contains(pc))
        {
            continue;
        }

        if (filter.hasPort && !filter.port.Contains(port))
        {
            continue;
        }

        if (filter.hasDirection)
        {
            if (filter.direction == TraceFilterDirection::Read && isWrite)
            {
                continue;
            }

            if (filter.direction == TraceFilterDirection::Write && !isWrite)
            {
                continue;
            }
        }

        if (!filter.text.empty() &&
            loweredText.find(filter.text) == std::string::npos)
        {
            continue;
        }

        return true;
    }

    return false;
}
