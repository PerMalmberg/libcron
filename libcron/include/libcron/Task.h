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

            void execute(std::chrono::system_clock::time_point now)
            {
                // Next Schedule is still the current schedule, check if execution was on time (within 1 second) 
                using namespace std::chrono_literals;
                was_executed_on_time = (now <= (next_schedule + 1s));
                
                last_run = now;
                task();
            }

            bool executed_on_time() const
            {
                return was_executed_on_time;
            }

            Task(const Task& other) = default;

            Task& operator=(const Task&) = default;

            bool calculate_next(std::chrono::system_clock::time_point from);

            bool operator>(const Task& other) const
            {
                return next_schedule > other.next_schedule;
            }

            bool is_expired(std::chrono::system_clock::time_point now) const;

            std::chrono::system_clock::duration
            time_until_expiry(std::chrono::system_clock::time_point now) const;

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
            bool was_executed_on_time = false;
            std::chrono::system_clock::time_point last_run = std::numeric_limits<std::chrono::system_clock::time_point>::min();
    };
}

inline bool operator==(const std::string &lhs, const libcron::Task &rhs)
{
    return lhs == rhs.get_name();
}

inline bool operator==(const libcron::Task &lhs, const std::string &rhs)
{
    return lhs.get_name() == rhs;
}

inline bool operator!=(const std::string &lhs, const libcron::Task &rhs)
{
    return !(lhs == rhs);
}

inline bool operator!=(const libcron::Task &lhs, const std::string &rhs)
{
    return !(lhs == rhs);
}
