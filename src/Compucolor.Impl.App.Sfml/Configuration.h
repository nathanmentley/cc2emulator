#pragma once

#include <fstream>
#include <map>

#include <Compucolor/Common/IConfiguration.h>

namespace Compucolor::Impl::App::Sfml
{
    // Reads key=value lines from ./app.config. This file lists plugin library
    // paths relative to the working directory; user preferences live in Settings.
    class Configuration: public Compucolor::Common::IConfiguration {
        public:
            Configuration();

            std::optional<std::string> GetSetting(std::string key) override;

        private:
            void LoadFile(const std::string& filename);

            std::map<std::string, std::string> _data;
    };
}