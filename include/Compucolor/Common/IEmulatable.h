#pragma once

#include <boost/json.hpp>

namespace Compucolor::Common
{
    class IEmulatable {
        public:
            virtual ~IEmulatable() = default;
    
            virtual void Start() = 0;
            virtual void Stop() = 0;
            virtual std::unique_ptr<boost::json::value> SaveState() = 0;
            virtual void LoadState(std::shared_ptr<boost::json::value>) = 0;
    };
}