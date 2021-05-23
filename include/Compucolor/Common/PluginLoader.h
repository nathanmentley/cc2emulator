#pragma once

#include <memory>
#include <string>

#include <boost/dll/import.hpp>
#include <boost/function.hpp>

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
                std::string pluginLibrary,
                std::shared_ptr<PluginLoader> pluginLoader
            )
            {
                auto creator = boost::dll::import_alias<std::shared_ptr<TPlugin>(PluginLoader*)>(
                    pluginLibrary,
                    "create",
                    boost::dll::load_mode::append_decorations
                );

                return creator(pluginLoader.get());
            };
    };
}