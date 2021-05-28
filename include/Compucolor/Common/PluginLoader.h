#pragma once

#include <memory>
#include <string>

#include <boost/dll/import.hpp>
#include <boost/function.hpp>

#include "IPlugin.h"

namespace Compucolor::Common
{
    template<typename TPlugin> 
    class PluginLoader {
        typedef std::shared_ptr<TPlugin> (pluginapi_create_t)();

        public:
            PluginLoader(
                std::string pluginLibrary
            )
            {
                _creator =
                    boost::dll::import_alias<pluginapi_create_t>(
                        pluginLibrary,
                        "create",
                        boost::dll::load_mode::append_decorations
                    );
            };

            std::shared_ptr<TPlugin> Create()
            {
                return _creator();
            };

        private:
            boost::function<pluginapi_create_t> _creator;
    };
}