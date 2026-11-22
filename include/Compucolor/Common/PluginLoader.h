#pragma once

#include <memory>
#include <optional>
#include <string>

#include <boost/dll/import.hpp>
#include <boost/function.hpp>

#include <Compucolor/Common/IPluginManager.h>

namespace Compucolor::Common
{
    // Loads one plugin shared library and caches its single created instance.
    // The library must export "create" with the signature shared_ptr<TPlugin>(IPluginManager*);
    // append_decorations lets configuration omit the platform library suffix.
    template<typename TPlugin>
    class PluginLoader {
        typedef std::shared_ptr<TPlugin>(pluginapi_create_t)(IPluginManager*);

        public:
            PluginLoader(
                std::string pluginLibrary
            ):
                _plugin({})
            {
                _creator =
                    boost::dll::import_alias<pluginapi_create_t>(
                        pluginLibrary,
                        "create",
                        boost::dll::load_mode::append_decorations
                    );
            };

            // Invokes the factory on first use with the manager so the plugin
            // can resolve its dependencies; later calls return the cached object.
            std::shared_ptr<TPlugin> Create(IPluginManager* manager)
            {
                if (!_plugin.has_value())
                {
                    _plugin = _creator(manager);
                }

                return _plugin.value();
            };

        private:
            boost::function<pluginapi_create_t> _creator;
            std::optional<std::shared_ptr<TPlugin>> _plugin;
    };
}
