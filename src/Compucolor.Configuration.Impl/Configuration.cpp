#include <Compucolor.Configuration.Impl/Configuration.h>

Compucolor::Configuration::Impl::Configuration::Configuration(
    std::shared_ptr<ILogger> logger
):
    _data(std::map<std::string, std::string>()),
    _logger(logger)
{
}

void Compucolor::Configuration::Impl::Configuration::LoadFile(std::string filename)
{
    _logger->LogTrace(
        "In Configuration, loading config file"//: %s",
        //filename.c_str()
    );

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
                std::string value = line.substr(split, line.length());
                /*
                _logger->LogTrace(
                    "In Configuration, Setting up a configuration setting with key: %s and value: %s",
                    key.c_str(),
                    value.c_str()
                );*/
            }
            else
            {
                /*_logger->LogWarning(
                    "In Configuration, Found unparsable configuration line",//: %s",
                    //line.c_str()
                );*/
            }
        }

        file.close();
    }
    else
    {
        _logger->LogTrace(
            "In Configuration, could not load file"//: %s",
            //filename.c_str()
        );
    }
}

std::optional<std::string> Compucolor::Configuration::Impl::Configuration::GetSetting(std::string key)
{
    if(_data.count(key))
    {
        std::string value = _data[key];

        _logger->LogTrace(
            "In Configuration, Found key: %s with value: %s",
            key.c_str(),
            value.c_str()
        );

        return value;
    }

    _logger->LogTrace(
        "In Configuration, requested setting key: %s not found",
        key.c_str()
    );

    return {};
}