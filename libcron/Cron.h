#pragma once

#include <string>
#include <chrono>
#include <queue>
#include <memory>
#include "Task.h"
#include "CronClock.h"

namespace libcron
{
    class Cron
    {
        public:

            explicit Cron(std::unique_ptr<ICronClock> clock = std::make_unique<LocalClock>())
                    : clock(std::move(clock))
            {
            }

            bool add_schedule(std::string name, const std::string& schedule, std::function<void()> work);

            size_t count() const
            {
                return tasks.size();
            }

            size_t
            execute_expired_tasks()
            {
                return execute_expired_tasks(clock->now());
            }

            size_t
            execute_expired_tasks(std::chrono::system_clock::time_point now);

            std::chrono::system_clock::duration
            time_until_next() const;

            std::shared_ptr<ICronClock> get_clock() const { return clock; }

        private:
            // Priority queue placing smallest (i.e. nearest in time) items on top.
            std::priority_queue<Task, std::vector<Task>, std::greater<>> tasks{};
            void print_queue(std::priority_queue<Task, std::vector<Task>, std::greater<>> queue);
            std::shared_ptr<ICronClock> clock{};
    };
}