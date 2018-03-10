#include <functional>
#include "Cron.h"

namespace libcron
{

    bool libcron::Cron::add_schedule(const std::string& schedule, std::function<void()> work)
    {
        auto cron = CronData::create(schedule);
        bool res = cron.is_valid();
        if (res)
        {
            CronSchedule s(cron);
            Task t(std::move(s), std::move(work));
            if (t.calculate_next())
            {
                items.emplace(t);
            }
        }

        return res;
    }

    bool libcron::Cron::has_expired_task(const std::chrono::system_clock::time_point now) const
    {
        return !items.empty() && items.top().is_expired(now);
    }

}