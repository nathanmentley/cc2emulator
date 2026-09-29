#include <boost/dll/alias.hpp>                          // for BOOST_DLL_ALIAS

#include "FloppyEmulator.h"

using Compucolor::Impl::Floppy::FloppyEmulator;

BOOST_DLL_ALIAS(FloppyEmulator::Creator, create);