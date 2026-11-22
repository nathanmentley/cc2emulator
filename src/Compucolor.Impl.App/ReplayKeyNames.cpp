#include <Compucolor/App/Replay/ReplayKeyNames.h>

#include <utility>

using Compucolor::Keyboard::CompucolorIIKey;

std::string Compucolor::Impl::App::Replay::ReplayKeyNames::ToString(const CompucolorIIKey key)
{
    switch (key)
    {
#define CC2_CASE(name) case CompucolorIIKey::name: return #name;
        CC2_REPLAY_KEYS(CC2_CASE)
#undef CC2_CASE
    }

    return "Unknown";
}

std::optional<CompucolorIIKey> Compucolor::Impl::App::Replay::ReplayKeyNames::FromString(
    const std::string& name
)
{
    static const std::pair<const char*, CompucolorIIKey> names[] = {
#define CC2_ENTRY(name) { #name, CompucolorIIKey::name },
        CC2_REPLAY_KEYS(CC2_ENTRY)
#undef CC2_ENTRY
    };

    for (const auto& [candidate, key] : names)
    {
        if (name == candidate)
        {
            return key;
        }
    }

    return std::nullopt;
}
