#pragma once

#include <string>
#include <string_view>

#include <boost/json.hpp>

namespace Compucolor::Impl::App::Replay
{
    // Deterministic state fingerprint. Object keys are sorted before hashing so
    // harmless changes to serialization order do not alter the digest.
    class StateHash {
        public:
            [[nodiscard]] static std::string OfState(const boost::json::value& state);

            [[nodiscard]] static std::string Sha256(std::string_view input);

            [[nodiscard]] static boost::json::value Canonicalize(
                const boost::json::value& value
            );
    };
}
