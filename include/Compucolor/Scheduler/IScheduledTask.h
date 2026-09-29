#pragma once

namespace Compucolor::Scheduler
{
    class IScheduledTask {
        public:
            virtual ~IScheduledTask() = default;

            virtual void Stop() = 0;

            virtual void Join() = 0;

            [[nodiscard]] virtual bool IsStopRequested() const = 0;
    };
}