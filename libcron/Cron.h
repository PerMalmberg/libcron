#pragma once

#include <string>
#include <chrono>
#include <queue>
#include <memory>
#include "Task.h"

namespace libcron
{
    class Cron
    {

        public:
            bool add_schedule(std::string name, const std::string& schedule, std::function<void()> work);

            size_t count() const
            {
                return tasks.size();
            }

            size_t
            execute_expired_tasks(std::chrono::system_clock::time_point now = std::chrono::system_clock::now());

            std::chrono::system_clock::duration
            time_until_next(std::chrono::system_clock::time_point now = std::chrono::system_clock::now()) const;

        private:
            // Priority queue placing smallest (i.e. nearest in time) items on top.
            std::priority_queue<Task, std::vector<Task>, std::greater<>> tasks{};
            void print_queue(std::priority_queue<Task, std::vector<Task>, std::greater<>> queue);
    };
}