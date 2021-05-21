#pragma once

#include <fstream>
#include <map>
#include <memory>

#include <Compucolor/Logger/ILogger.h>

#include <Compucolor/Configuration/IConfiguration.h>

namespace Compucolor::Impl::Configuration
{
    class Configuration: public Compucolor::Configuration::IConfiguration {
        public:
            Configuration(
                std::shared_ptr<Logger::ILogger> logger
            );

            virtual void LoadFile(std::string filename) override;

            virtual std::optional<std::string> GetSetting(std::string key) override;

        private:
            std::map<std::string, std::string> _data;
            std::shared_ptr<Logger::ILogger> _logger;
    };
}