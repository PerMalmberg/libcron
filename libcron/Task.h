#pragma once

#include <functional>
#include "CronData.h"
#include "CronSchedule.h"
#include <chrono>
#include <utility>

namespace libcron
{
    class Task
    {
        public:

            Task(std::string name, const CronSchedule schedule, std::function<void()> task)
                    : name(std::move(name)), schedule(std::move(schedule)), task(std::move(task))
            {
            }

            void execute() const
            {
                task();
            }

            Task(const Task& other) = default;

            Task& operator=(const Task&) = default;

            bool calculate_next(std::chrono::system_clock::time_point from = std::chrono::system_clock::now());

            bool operator>(const Task& other) const
            {
                return next_schedule > other.next_schedule;
            }

            bool is_expired(std::chrono::system_clock::time_point now = std::chrono::system_clock::now()) const;

            std::chrono::system_clock::duration
            time_until_expiry(std::chrono::system_clock::time_point now = std::chrono::system_clock::now()) const;

            std::string get_name() const
            {
                return name;
            }

            std::string get_status(std::chrono::system_clock::time_point now) const;

        private:
            std::string name;
            CronSchedule schedule;
            std::chrono::system_clock::time_point next_schedule;
            std::function<void()> task;
            bool valid = false;
    };
}