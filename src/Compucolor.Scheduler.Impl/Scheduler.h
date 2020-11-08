#pragma once

#include <Compucolor.Core/ThirdParty/nameof.hpp>

#include <Compucolor.Logger/ILogger.h>

#include <Compucolor.Scheduler/IScheduler.h>

class Scheduler: public IScheduler {
    public:
        Scheduler(
            std::shared_ptr<ILogger> logger
        );

        virtual std::future<int> RunOnce(uint64_t afterNanoseconds, std::function<int()> logic) override;

        virtual std::thread SetupReoccuringTask(uint64_t everyNanoseconds, std::function<int()> logic) override;

    private:
        std::shared_ptr<ILogger> _logger;
};