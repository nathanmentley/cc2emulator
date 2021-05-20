#pragma once

#include "IEmulatable.h"

namespace Compucolor::Common
{
    class ISystem: public IEmulatable {
        public:
            virtual ~ISystem() {}
    };
}