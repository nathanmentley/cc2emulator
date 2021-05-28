#pragma once

#include <memory>
#include <string>

#include "IPluginManager.h"
#include "PluginLoader.h"

#include "../Memory/IMemory.h"

namespace Compucolor::Common
{
    class PluginManager: public IPluginManager {
        public:
            PluginManager(
                std::string intel8080Library,
                std::string memoryLibrary
            ):
                _intel8080PluginLoader(
                    std::shared_ptr<PluginLoader<Intel8080::IIntel8080Emulator>>(
                        new PluginLoader<Intel8080::IIntel8080Emulator>(intel8080Library)
                    )
                ),
                _memoryPluginLoader(
                    std::shared_ptr<PluginLoader<Memory::IMemory>>(
                        new PluginLoader<Memory::IMemory>(memoryLibrary)
                    )
                )
            {
            };

            virtual std::shared_ptr<Intel8080::IIntel8080Emulator> GetIntel8080() override {
                return _intel8080PluginLoader->Create(this);
            }

            virtual std::shared_ptr<Memory::IMemory> GetMemory() override {
                return _memoryPluginLoader->Create(this);
            }

        private:
            std::shared_ptr<PluginLoader<Intel8080::IIntel8080Emulator>> _intel8080PluginLoader;
            std::shared_ptr<PluginLoader<Memory::IMemory>> _memoryPluginLoader;
    };
}