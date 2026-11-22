#pragma once

namespace Compucolor::Common
{
    // Reserved marker interface for CPU implementations.
    // The Intel 8080 wrapper currently uses IEmulatable/IResetable instead.
    class ICpu {
        public:
            virtual ~ICpu() = default;
    };
}
