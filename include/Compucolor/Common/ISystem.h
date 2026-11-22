#pragma once

#include <Compucolor/Common/IEmulatable.h>

namespace Compucolor::Common
{
    // Marker interface for a complete emulated system rather than a single component.
    class ISystem: public IEmulatable {
        public:
            virtual ~ISystem() = default;
    };
}
