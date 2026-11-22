#pragma once

namespace Compucolor::Scheduler
{
    // Handle for a recurring scheduler task. The destructor stops and joins it.
    class IScheduledTask {
        public:
            virtual ~IScheduledTask() = default;

            // Requests stop and wakes the waiting worker; safe to call repeatedly.
            virtual void Stop() = 0;

            // Blocks until the worker thread exits.
            virtual void Join() = 0;

            [[nodiscard]] virtual bool IsStopRequested() const = 0;
    };
}
