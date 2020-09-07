#pragma once

#include <functional>
#include <chrono>
#include <utility>
#include "CronData.h"
#include "CronSchedule.h"

namespace libcron
{
    class TaskInterface
    {
        public:
            virtual ~TaskInterface() {}
            virtual std::chrono::system_clock::duration get_delay() const 
            { 
                using namespace std::chrono_literals;
                return std::chrono::system_clock::duration(-1s);
            };
    };

    class Task : public TaskInterface
    {
        public:
            using TaskFunction = std::function<void(const TaskInterface&)>;

            class TaskProxy
            {
                public:
                TaskProxy(TaskFunction task) : task(std::move(task)) {}

                void operator() (const TaskInterface& i)
                {
                    task(i);
                }

                private:
                TaskFunction task;
            };

            Task(std::string name, const CronSchedule schedule, TaskProxy task)
                    : name(std::move(name)), schedule(std::move(schedule)), task(std::move(task))
            {
            }

            void execute(std::chrono::system_clock::time_point now)
            {
                // Next Schedule is still the current schedule, check if execution was on time (within 1 second) 
                delay = now - next_schedule;

                last_run = now;
                task(*this);
            }

            std::chrono::system_clock::duration get_delay() const
            {
                return delay;
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
            std::chrono::system_clock::duration delay = std::chrono::seconds(-1);
            TaskProxy task;
            bool valid = false;
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
