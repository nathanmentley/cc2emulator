#pragma once

#include <optional>
#include <string>

namespace Compucolor::Configuration
{
    class IConfiguration {
        public:
            virtual ~IConfiguration() {}
    
            virtual void LoadFile(std::string filename) = 0;
    
            virtual std::optional<std::string> GetSetting(std::string key) = 0;
    };
}