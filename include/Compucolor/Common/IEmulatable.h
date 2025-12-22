#pragma once

namespace Compucolor::Common
{
    class IEmulatable {
        public:
            virtual ~IEmulatable() = default;
    
            virtual void Start() = 0;
            virtual void Stop() = 0;
    };
}