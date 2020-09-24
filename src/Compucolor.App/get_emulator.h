#pragma once

#include <memory>

#include <fruit/fruit.h>

#include "Compucolor.Core/IDisplay.h"

#include "Compucolor.Core/IMemory.h"
#include "Compucolor.Core/ByteArrayMemory.h"

#include "Compucolor.Intel8080/IIntel8080Emulator.h"
#include "Compucolor.Intel8080.Impl/Intel8080Emulator.h"

#include "Compucolor.Crt/ICrtEmulator.h"
#include "Compucolor.Crt.Impl/CrtEmulator.h"

#include "Compucolor.Compucolor/ICompucolorEmulator.h"
#include "Compucolor.Compucolor.Impl/CompucolorEmulator.h"

std::unique_ptr<ICompucolorEmulator> get_emulator();