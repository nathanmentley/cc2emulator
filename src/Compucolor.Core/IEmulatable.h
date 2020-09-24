
#pragma once

class IEmulatable {
    public:
        virtual ~IEmulatable() {}

        virtual void Start() = 0;
        virtual void Stop() = 0;
};