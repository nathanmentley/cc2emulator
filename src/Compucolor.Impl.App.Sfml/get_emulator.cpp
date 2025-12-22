#include "get_emulator.h"

#include <memory>

std::unique_ptr<Compucolor::CompucolorEmulatorProcessor> Compucolor::Impl::App::Sfml::get_emulator(
    const std::shared_ptr<Common::IPluginManager>& manager
)
{
    return std::make_unique<CompucolorEmulatorProcessor>(manager->GetCompucolor());
}