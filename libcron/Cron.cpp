#include <functional>
#include "Cron.h"

bool libcron::Cron::add_schedule(const std::string &schedule, std::function<void()> work)
{
    auto cron = CronData::create(schedule);
    bool res = cron.is_valid();
    if (res)
    {
        items.emplace(Task(CronSchedule(cron), std::move(work)));
    }

    return res;
}
