#pragma once

#include <functional>
#include "CronData.h"
#include "CronSchedule.h"

namespace libcron
{
    class Task
    {
        public:

            Task(CronSchedule schedule, std::function<void()> task)
                    : schedule(std::move(schedule))//, task(std::move(task))
            {
            }

            Task(const Task&) = default;

            Task& operator=(const Task&) = default;

            bool operator<(const Task& other) const
            {
                return false;
            }

        private:
            CronSchedule schedule;
            //std::function<void()> task;
    };
}