#pragma once

#include <functional>
#include "CronTime.h"

namespace libcron
{
    class Task
    {
    public:

        Task(CronTime time, std::function<void()> task)
                : time(std::move(time)), task(std::move(task))
        {
        }

        bool operator<(const Task& other) const
        {
            return time < other.time;
        }

    private:
        CronTime time{};
        std::function<void()> task;
    };
}