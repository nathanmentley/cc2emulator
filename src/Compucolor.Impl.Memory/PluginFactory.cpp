#include <Compucolor/Common/PluginLoader.h>

#include "VectorMemory.h"

std::shared_ptr<Compucolor::Memory::IMemory> create(
    std::shared_ptr<Compucolor::Common::PluginLoader> pluginLoader
)
{
    return std::shared_ptr<Compucolor::Memory::IMemory>(
        new Compucolor::Impl::Memory::VectorMemory()
    );
}