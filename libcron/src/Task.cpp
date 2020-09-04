#include "libcron/Task.h"

using namespace std::chrono;

namespace libcron
{

    bool Task::calculate_next(std::chrono::system_clock::time_point from)
    {
        auto result = schedule.calculate_from(from);

        // In case the calculation fails, the task will no longer expire.
        valid = std::get<0>(result);
        if (valid)
        {
            next_schedule = std::get<1>(result);

            // Make sure that the task is allowed to run.
            last_run = next_schedule - 1s;
        }

        return valid;
    }

    bool Task::is_expired(std::chrono::system_clock::time_point now) const
    {
        return valid && now >= last_run && time_until_expiry(now) == 0s;
    }

    std::chrono::system_clock::duration Task::time_until_expiry(std::chrono::system_clock::time_point now) const
    {
        system_clock::duration d{};

        // Explicitly return 0s instead of a possibly negative duration when it has expired.
        if (now >= next_schedule)
        {
            d = 0s;
        }
        else
        {
            d = next_schedule - now;
        }

        return d;
    }

    std::string Task::get_status(std::chrono::system_clock::time_point now) const
    {
        std::string s = "'";
        s+= get_name();
        s += "' expires in ";
        s += std::to_string(duration_cast<milliseconds>(time_until_expiry(now)).count());
        s += "ms => ";

        auto dt = CronSchedule::to_calendar_time(next_schedule);
        s+= std::to_string(dt.year) + "-";
        s+= std::to_string(dt.month) + "-";
        s+= std::to_string(dt.day) + " ";
        s+= std::to_string(dt.hour) + ":";
        s+= std::to_string(dt.min) + ":";
        s+= std::to_string(dt.sec);
        return s;
    }
}