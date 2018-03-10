#include <catch.hpp>
#include <libcron/Cron.h>
#include <thread>

using namespace libcron;
using namespace std::chrono;

std::string create_schedule_expiring_in(hours h, minutes m, seconds s)
{
    auto now = system_clock::now() + h + m + s;
    auto dt = CronSchedule::to_calendar_time(now);

    std::string res = "";
    res += std::to_string(dt.sec) + " ";
    res += std::to_string(dt.min) + " ";
    res += std::to_string(dt.hour) + " * * *";

    return res;
}


SCENARIO("Adding a task")
{
    GIVEN("A Cron instance with no task")
    {
        Cron c;

        THEN("Starts with no task")
        {
            REQUIRE(c.count() == 0);
        }

        WHEN("Adding a task that runs every second")
        {
            REQUIRE(c.add_schedule("* * * * * *",
                                   []()
                                   {
                                       return;
                                   })
            );

            THEN("Count is 1 and task was not expired two seconds ago")
            {
                REQUIRE(c.count() == 1);
                REQUIRE_FALSE(c.has_expired_task(system_clock::now() - 2s));
            }
            AND_THEN("Task is expired when calculating based on current time")
            {
                THEN("Task is expired")
                {
                    REQUIRE(c.has_expired_task());
                }
            }
        }
    }
}

SCENARIO("Adding a task that expires in the future")
{
    GIVEN("A Cron instance with task expiring in 3 seconds")
    {
        Cron c;
        REQUIRE(c.add_schedule(create_schedule_expiring_in(hours{0}, minutes{0}, seconds{3}),
                               []()
                               {
                                   return;
                               })
        );

        THEN("Not yet expired")
        {
            REQUIRE_FALSE(c.has_expired_task());
        }
        AND_WHEN("When waiting one second")
        {
            std::this_thread::sleep_for(1s);
            THEN("Task has not yet expired")
            {
                REQUIRE_FALSE(c.has_expired_task());
            }
        }
        AND_WHEN("When waiting three seconds")
        {
            std::this_thread::sleep_for(3s);
            THEN("Task has expired")
            {
                REQUIRE(c.has_expired_task());
            }
        }
    }
}

