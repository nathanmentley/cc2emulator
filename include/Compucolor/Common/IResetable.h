#pragma once

namespace Compucolor::Common
{
    class IResetable {
        public:
            virtual ~IResetable() {}
    
            virtual void Reset() = 0;
    };
}