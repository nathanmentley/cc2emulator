#pragma once

#include <optional>
#include <string>

#include <Compucolor/Common/IPlugin.h>

namespace Compucolor::Common
{
    // Key/value application configuration. The SFML host reads plugin library
    // paths from app.config through this interface; user preferences are stored
    // separately by Settings.
    class IConfiguration: public Common::IPlugin {
        public:
            virtual ~IConfiguration() = default;

            // Returns nullopt when the key is absent.
            virtual std::optional<std::string> GetSetting(std::string key) = 0;
    };
}
