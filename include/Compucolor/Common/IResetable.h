#pragma once

namespace Compucolor::Common
{
    // Implemented by devices whose state can be reset without recreating them.
    class IResetable {
        public:
            virtual ~IResetable() = default;

            // Clears device state to its power-on condition.
            virtual void Reset() = 0;
    };
}
