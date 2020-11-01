#pragma once

#include <functional>
#include <future>
#include <string>
#include <thread>

class IScheduler {
    public:
        virtual ~IScheduler() {}

        virtual std::future<int> RunOnce(uint64_t afterNanoseconds, std::function<int()> logic) = 0;

        virtual std::thread SetupReoccuringTask(uint64_t everyNanoseconds, std::function<int()> logic) = 0;
};