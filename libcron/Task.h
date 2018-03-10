#pragma once

#include <functional>
#include "CronData.h"
#include "CronSchedule.h"
#include <chrono>

namespace libcron
{
    class Task
    {
        public:

            Task(CronSchedule schedule, std::function<void()> task)
                    : schedule(std::move(schedule)), task(std::move(task))
            {
            }

            Task(const Task&) = default;

            Task& operator=(const Task&) = default;

            bool calculate_next(std::chrono::system_clock::time_point from = std::chrono::system_clock::now());

            bool operator<(const Task& other) const
            {
                return next_schedule < other.next_schedule;
            }

            bool is_expired(std::chrono::system_clock::time_point now = std::chrono::system_clock::now()) const
            {
                return now >= next_schedule;
            }

        private:
            CronSchedule schedule;
            std::chrono::system_clock::time_point next_schedule;
            std::function<void()> task;
    };
}