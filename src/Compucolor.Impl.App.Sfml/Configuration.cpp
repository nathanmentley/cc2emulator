#include "Configuration.h"

Compucolor::Impl::App::Sfml::Configuration::Configuration():
    _data(std::map<std::string, std::string>())
{
    LoadFile("app.config");
}

void Compucolor::Impl::App::Sfml::Configuration::LoadFile(std::string filename)
{
    std::ifstream file(filename);
    if (file.is_open())
    {
        std::string line;
 
        while (std::getline(file, line))
        {
            size_t split = line.find_first_of("=");

            if (split != std::string::npos)
            {
                std::string key = line.substr(0, split);
                std::string value = line.substr(split + 1, line.length());

                _data[key] = value;
            }
        }

        file.close();
    }
}

std::optional<std::string> Compucolor::Impl::App::Sfml::Configuration::GetSetting(std::string key)
{
    if(_data.count(key))
    {
        std::string value = _data[key];

        return value;
    }

    return {};
}