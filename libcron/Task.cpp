#include "Task.h"

namespace libcron
{

    bool Task::calculate_next(std::chrono::system_clock::time_point from)
    {
        auto result = schedule.calculate_from(from);
        auto res = std::get<0>(result);
        if(res)
        {
            next_schedule = std::get<1>(result);
        }

        return res;
    }
}