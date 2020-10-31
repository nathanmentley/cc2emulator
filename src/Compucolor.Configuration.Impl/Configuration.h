#pragma once

#include <fstream>
#include <map>
#include <memory>

#include <Compucolor.Core/ThirdParty/nameof.hpp>

#include <Compucolor.Logger/ILogger.h>

#include <Compucolor.Configuration/IConfiguration.h>

class Configuration: public IConfiguration {
    public:
        Configuration(
            std::shared_ptr<ILogger> logger
        );

        virtual void LoadFile(std::string filename) override;

        virtual std::optional<std::string> GetSetting(std::string key) override;

    private:
        std::map<std::string, std::string> _data;
        std::shared_ptr<ILogger> _logger;
};