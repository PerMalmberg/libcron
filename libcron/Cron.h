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

            size_t count() const
            {
                return items.size();
            }

            bool has_expired_task(std::chrono::system_clock::time_point now = std::chrono::system_clock::now()) const;

        private:
            std::priority_queue<Task> items{};
    };
}