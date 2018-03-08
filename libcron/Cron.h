#pragma once

#include <string>
#include <chrono>
#include <queue>
#include "Task.h"

namespace libcron
{
    class Cron
    {

    public:
        bool add_schedule(const std::string& schedule, std::function<void()> work);
    private:
        std::priority_queue<Task> items{};
    };
}