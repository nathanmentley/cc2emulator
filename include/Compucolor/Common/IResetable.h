#pragma once

namespace Compucolor::Common
{
    class IResetable {
        public:
            virtual ~IResetable() = default;
    
            virtual void Reset() = 0;
    };
}