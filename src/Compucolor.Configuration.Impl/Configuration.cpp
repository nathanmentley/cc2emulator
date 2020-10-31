#include <Compucolor.Configuration.Impl/Configuration.h>

Configuration::Configuration(
    std::shared_ptr<ILogger> logger
):
    _data(std::map<std::string, std::string>()),
    _logger(logger)
{
}

void Configuration::LoadFile(std::string filename)
{
    _logger->LogTrace(
        "In %s, loading config file",//: %s",
        NAMEOF_TYPE(Configuration)//,
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
                    "In %s, Setting up a configuration setting with key: %s and value: %s",
                    NAMEOF_TYPE(Configuration),
                    key.c_str(),
                    value.c_str()
                );*/
            }
            else
            {
                /*_logger->LogWarning(
                    "In %s, Found unparsable configuration line",//: %s",
                    NAMEOF_TYPE(Configuration)//,
                    //line.c_str()
                );*/
            }
        }

        file.close();
    }
    else
    {
        _logger->LogTrace(
            "In %s, could not load file",//: %s",
            NAMEOF_TYPE(Configuration)//,
            //filename.c_str()
        );
    }
}

std::optional<std::string> Configuration::GetSetting(std::string key)
{
    if(_data.count(key))
    {
        std::string value = _data[key];

        _logger->LogTrace(
            "In %s, Found key: %s with value: %s",
            NAMEOF_TYPE(Configuration),
            key.c_str(),
            value.c_str()
        );

        return value;
    }

    _logger->LogTrace(
        "In %s, requested setting key: %s not found",
        NAMEOF_TYPE(Configuration),
        key.c_str()
    );

    return {};
}