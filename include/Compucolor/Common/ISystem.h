#pragma once

#include <Compucolor/Common/IEmulatable.h>

namespace Compucolor::Common
{
    class ISystem: public IEmulatable {
        public:
            virtual ~ISystem() = default;
    };
}