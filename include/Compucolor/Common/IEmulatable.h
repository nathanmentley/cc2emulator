#pragma once

#include <boost/json.hpp>

namespace Compucolor::Common
{
    // Lifecycle and JSON state contract shared by emulated components and systems.
    class IEmulatable {
        public:
            virtual ~IEmulatable() = default;

            // Starts any threads or recurring tasks owned by the component.
            virtual void Start() = 0;

            // Stops and joins those tasks. Must be safe to call during shutdown.
            virtual void Stop() = 0;

            // Returns an owned snapshot of the current state.
            // Implementations must return a non-null value; nullptr means an error.
            virtual std::unique_ptr<boost::json::value> SaveState() = 0;

            // Restores state from a snapshot. Implementations validate the JSON
            // before mutating state and throw on invalid or missing fields.
            virtual void LoadState(std::shared_ptr<boost::json::value>) = 0;
    };
}
