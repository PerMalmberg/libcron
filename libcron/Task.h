#pragma once

#include <functional>
#include "CronData.h"

namespace libcron
{
    class Task
    {
    public:

        Task(CronData time, std::function<void()> task)
                : time(std::move(time)), task(std::move(task))
        {
        }

        bool operator<(const Task& other) const
        {
            return false;
        }

    private:
        CronData time{};
        std::function<void()> task;
    };
}