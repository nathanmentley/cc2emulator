#pragma once

#include <memory>
#include <string>

namespace Compucolor::Common
{
    class PluginLoader {
        public:
            PluginLoader()
            {
            };

            template<typename TPlugin> void RegisterPlugin(
                std::string pluginFilePath
            )
            {

            };

            template<typename TPlugin> std::shared_ptr<TPlugin> Create(
                std::shared_ptr<PluginLoader> pluginLoader
            )
            {
                return std::shared_ptr<TPlugin>(
                    0
                );
            };
    };
}