#pragma once

#include <fstream>
#include <map>

#include <Compucolor/Common/IConfiguration.h>

namespace Compucolor::Impl::App::Sfml
{
    class Configuration: public Compucolor::Common::IConfiguration {
        public:
            Configuration();

            virtual std::optional<std::string> GetSetting(std::string key) override;

        private:
            void LoadFile(std::string filename);

            std::map<std::string, std::string> _data;
    };
}