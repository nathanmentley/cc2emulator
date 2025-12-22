#pragma once

#include <memory>

#include <Compucolor/CompucolorEmulatorProcessor.h>
#include <Compucolor/Common/IPluginManager.h>

namespace Compucolor::Impl::App::Sfml
{
    std::unique_ptr<CompucolorEmulatorProcessor> get_emulator(const std::shared_ptr<Common::IPluginManager> &manager);
}