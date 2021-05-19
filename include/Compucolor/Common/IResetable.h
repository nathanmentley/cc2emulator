#pragma once

class IResetable {
    public:
        virtual ~IResetable() {}

        virtual void Reset() = 0;
};