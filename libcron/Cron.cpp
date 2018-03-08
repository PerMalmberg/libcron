//
// Created by permal on 3/8/18.
//


#include <functional>
#include "Cron.h"

bool libcron::Cron::add_schedule(const std::string &schedule, std::function<void()> work)
{
    auto cron = CronTime::create(schedule);
    bool res = cron.is_valid();
    if (res)
    {
        items.emplace(Task(cron, std::move(work)));
    }

    return res;
}
