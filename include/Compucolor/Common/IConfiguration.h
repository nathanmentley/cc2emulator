#pragma once

#include <optional>
#include <string>

#include <Compucolor/Common/IPlugin.h>

namespace Compucolor::Common
{
    class IConfiguration: public Common::IPlugin {
        public:
            virtual ~IConfiguration() {}

            virtual std::optional<std::string> GetSetting(std::string key) = 0;
    };
}