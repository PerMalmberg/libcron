#include "libcron/TaskContext.h"
#include "libcron/Task.h"
#include "libcron/Cron.h"


namespace libcron
{
    std::chrono::system_clock::duration TaskContext::get_delay()
    {
        using namespace std::chrono_literals;
        std::chrono::system_clock::duration d = -1s;

        if (task)
        {
            d = task->get_delay();
        }

        return d;
    }
}
