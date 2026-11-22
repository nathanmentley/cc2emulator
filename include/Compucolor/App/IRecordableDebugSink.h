#pragma once

namespace Compucolor::Impl::App
{
    // Optional capability for sinks that should follow the hub's master
    // recording switch. IDebugSink deliberately has no setter because the
    // CPU monitor must keep receiving state snapshots while recording is off.
    class IRecordableDebugSink {
        public:
            virtual ~IRecordableDebugSink() = default;

            virtual void SetEnabled(bool enabled) = 0;
    };
}
