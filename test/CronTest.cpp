#include <catch.hpp>
#include <libcron/include/libcron/Cron.h>
#include <libcron/externals/date/include/date/date.h>
#include <thread>
#include <iostream>

using namespace libcron;
using namespace std::chrono;
using namespace date;

std::string create_schedule_expiring_in(std::chrono::system_clock::time_point now, hours h, minutes m, seconds s)
{
    now = now + h + m + s;
    auto dt = CronSchedule::to_calendar_time(now);

    std::string res{};
    res += std::to_string(dt.sec) + " ";
    res += std::to_string(dt.min) + " ";
    res += std::to_string(dt.hour) + " * * ?";

    return res;
}

SCENARIO("Adding a task")
{
    GIVEN("A Cron instance with no task")
    {
        Cron<> c;
        auto expired = false;

        THEN("Starts with no task")
        {
            REQUIRE(c.count() == 0);
        }

        WHEN("Adding a task that runs every second")
        {
            REQUIRE(c.add_schedule("A task", "* * * * * ?",
                                   [&expired](auto&)
                                   {
                                       expired = true;
                                   })
            );

            THEN("Count is 1 and task was not expired two seconds ago")
            {
                REQUIRE(c.count() == 1);
                c.tick(c.get_clock().now() - 2s);
                REQUIRE_FALSE(expired);
            }
            AND_THEN("Task is expired when calculating based on current time")
            {
                c.tick();
                THEN("Task is expired")
                {
                    REQUIRE(expired);
                }
            }
        }
    }
}

SCENARIO("Adding a task that expires in the future")
{
    GIVEN("A Cron instance with task expiring in 3 seconds")
    {
        auto expired = false;

        Cron<> c;
        REQUIRE(c.add_schedule("A task",
                               create_schedule_expiring_in(c.get_clock().now(), hours{0}, minutes{0}, seconds{3}),
                               [&expired](auto&)
                               {
                                   expired = true;
                               })
        );

        THEN("Not yet expired")
        {
            REQUIRE_FALSE(expired);
        }
        AND_WHEN("When waiting one second")
        {
            std::this_thread::sleep_for(1s);
            c.tick();
            THEN("Task has not yet expired")
            {
                REQUIRE_FALSE(expired);
            }
        }
        AND_WHEN("When waiting three seconds")
        {
            std::this_thread::sleep_for(3s);
            c.tick();
            THEN("Task has expired")
            {
                REQUIRE(expired);
            }
        }
    }
}

SCENARIO("Get delay using Task-Information")
{
    using namespace std::chrono_literals;

    GIVEN("A Cron instance with one task expiring in  2 seconds, but taking 3 seconds to execute")
    {
        auto _2_second_expired = 0;
        auto _delay = std::chrono::system_clock::duration(-1s);

        Cron<> c;
        REQUIRE(c.add_schedule("Two",
                               "*/2 * * * * ?",
                               [&_2_second_expired, &_delay](auto& i)
                               {
                                   _2_second_expired++;
                                   _delay = i.get_delay();
                                   std::this_thread::sleep_for(3s);
                               })
        );
        THEN("Not yet expired")
        {
            REQUIRE_FALSE(_2_second_expired);
            REQUIRE(_delay <= 0s);
        }
        WHEN("Exactly schedule task")
        {
            while (_2_second_expired == 0)
                c.tick();

            THEN("Task should have expired within a valid time")
            {
                REQUIRE(_2_second_expired == 1);
                REQUIRE(_delay <= 1s);
            }
            AND_THEN("Executing another tick again, leading to execute task again immediatly, but not on time as execution has taken 3 seconds.")
            {
                c.tick();
                REQUIRE(_2_second_expired == 2);
                REQUIRE(_delay >= 1s);
            }
        }
    }
}

SCENARIO("Task priority")
{
    GIVEN("A Cron instance with two tasks expiring in 3 and 5 seconds, added in 'reverse' order")
    {
        auto _3_second_expired = 0;
        auto _5_second_expired = 0;


        Cron<> c;
        REQUIRE(c.add_schedule("Five",
                               create_schedule_expiring_in(c.get_clock().now(), hours{0}, minutes{0}, seconds{5}),
                               [&_5_second_expired](auto&)
                               {
                                   _5_second_expired++;
                               })
        );

        REQUIRE(c.add_schedule("Three",
                               create_schedule_expiring_in(c.get_clock().now(), hours{0}, minutes{0}, seconds{3}),
                               [&_3_second_expired](auto&)
                               {
                                   _3_second_expired++;
                               })
        );

        THEN("Not yet expired")
        {
            REQUIRE_FALSE(_3_second_expired);
            REQUIRE_FALSE(_5_second_expired);
        }

        WHEN("Waiting 1 seconds")
        {
            std::this_thread::sleep_for(1s);
            c.tick();

            THEN("Task has not yet expired")
            {
                REQUIRE(_3_second_expired == 0);
                REQUIRE(_5_second_expired == 0);
            }
        }
        AND_WHEN("Waiting 3 seconds")
        {
            std::this_thread::sleep_for(3s);
            c.tick();

            THEN("3 second task has expired")
            {
                REQUIRE(_3_second_expired == 1);
                REQUIRE(_5_second_expired == 0);
            }
        }
        AND_WHEN("Waiting 5 seconds")
        {
            std::this_thread::sleep_for(5s);
            c.tick();

            THEN("3 and 5 second task has expired")
            {
                REQUIRE(_3_second_expired == 1);
                REQUIRE(_5_second_expired == 1);
            }
        }
        AND_WHEN("Waiting based on the time given by the Cron instance")
        {
            auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(c.time_until_next());
            std::this_thread::sleep_for(c.time_until_next());
            c.tick();

            THEN("3 second task has expired")
            {
                REQUIRE(_3_second_expired == 1);
                REQUIRE(_5_second_expired == 0);
            }
        }
        AND_WHEN("Waiting based on the time given by the Cron instance")
        {
            std::this_thread::sleep_for(c.time_until_next());
            REQUIRE(c.tick() == 1);

            std::this_thread::sleep_for(c.time_until_next());
            REQUIRE(c.tick() == 1);

            THEN("3 and 5 second task has each expired once")
            {
                REQUIRE(_3_second_expired == 1);
                REQUIRE(_5_second_expired == 1);
            }
        }
    }
}

class TestClock
        : public ICronClock
{
    public:
        std::chrono::system_clock::time_point now() const override
        {
            return current_time;
        }

        std::chrono::seconds utc_offset(std::chrono::system_clock::time_point) const override
        {
            return 0s;
        }

        void add(system_clock::duration time)
        {
            current_time += time;
        }

        void set(system_clock::time_point new_time)
        {
            current_time = new_time;
        }

    private:
        system_clock::time_point current_time = system_clock::now();

};

SCENARIO("Clock changes")
{
    GIVEN("A Cron instance with a single task expiring every hour")
    {
        Cron<TestClock> c{};
        auto& clock = c.get_clock();

        // Midnight
        clock.set(sys_days{2018_y / 05 / 05});

        // Every hour
        REQUIRE(c.add_schedule("Clock change task", "0 0 * * * ?", [](auto&)
        {
        })
        );

        // https://linux.die.net/man/8/cron

        WHEN("Clock changes <3h forward")
        {
            THEN("Task expires accordingly")
            {
                REQUIRE(c.tick() == 1);
                clock.add(minutes{30}); // 00:30
                REQUIRE(c.tick() == 0);
                clock.add(minutes{30}); // 01:00
                REQUIRE(c.tick() == 1);
                REQUIRE(c.tick() == 0);
                REQUIRE(c.tick() == 0);
                clock.add(minutes{30}); // 01:30
                REQUIRE(c.tick() == 0);
                clock.add(minutes{15}); // 01:45
                REQUIRE(c.tick() == 0);
                clock.add(minutes{15}); // 02:00
                REQUIRE(c.tick() == 1);
            }
        }
        AND_WHEN("Clock is moved forward >= 3h")
        {
            THEN("Task are rescheduled, not run")
            {
                REQUIRE(c.tick() == 1);
                clock.add(hours{3}); // 03:00
                REQUIRE(c.tick() == 1); // Rescheduled
                clock.add(minutes{15}); // 03:15
                REQUIRE(c.tick() == 0);
                clock.add(minutes{45}); // 04:00
                REQUIRE(c.tick() == 1);
            }
        }
        AND_WHEN("Clock is moved back <3h")
        {
            THEN("Tasks retain their last scheduled time and are prevented from running twice")
            {
                REQUIRE(c.tick() == 1);
                clock.add(-hours{1}); // 23:00
                REQUIRE(c.tick() == 0);
                clock.add(-hours{1}); // 22:00
                REQUIRE(c.tick() == 0);
                clock.add(hours{3}); // 1:00
                REQUIRE(c.tick() == 1);
            }
        }
        AND_WHEN("Clock is moved back >3h")
        {
            THEN("Tasks are rescheduled")
            {
                REQUIRE(c.tick() == 1);
                clock.add(-hours{3}); // 21:00
                REQUIRE(c.tick() == 1);
                REQUIRE(c.tick() == 0);
                clock.add(hours{1}); // 22:00
                REQUIRE(c.tick() == 1);
            }
        }
    }
}

SCENARIO("Multiple ticks per second")
{
    Cron<TestClock> c{};
    auto& clock = c.get_clock();

    auto now = sys_days{2018_y / 05 / 05};
    clock.set(now);

    int run_count = 0;

    // Every 10 seconds
    REQUIRE(c.add_schedule("Clock change task", "*/10 0 * * * ?", [&run_count](auto&)
    {
        run_count++;
    })
    );

    c.tick(now);

    REQUIRE(run_count == 1);

    WHEN("Many ticks during one seconds")
    {
        for(auto i = 0; i < 10; ++i)
        {
            clock.add(std::chrono::microseconds{1});
            c.tick();
        }

        THEN("Run count has not increased")
        {
            REQUIRE(run_count == 1);
        }

    }

}

SCENARIO("Tasks can be added and removed from the scheduler")
{
    GIVEN("A Cron instance with no task")
    {
        Cron<> c;
        auto expired = false;

        WHEN("Adding 5 tasks that runs every second")
        {
            REQUIRE(c.add_schedule("Task-1", "* * * * * ?",
                                   [&expired](auto&)
                                   {
                                       expired = true;
                                   })
            );

            REQUIRE(c.add_schedule("Task-2", "* * * * * ?",
                                   [&expired](auto&)
                                   {
                                       expired = true;
                                   })
            );

            REQUIRE(c.add_schedule("Task-3", "* * * * * ?",
                                   [&expired](auto&)
                                   {
                                       expired = true;
                                   })
            );

            REQUIRE(c.add_schedule("Task-4", "* * * * * ?",
                                   [&expired](auto&)
                                   {
                                       expired = true;
                                   })
            );

            REQUIRE(c.add_schedule("Task-5", "* * * * * ?",
                                   [&expired](auto&)
                                   {
                                       expired = true;
                                   })
            );

            THEN("Count is 5")
            {
                REQUIRE(c.count() == 5);
            }
            AND_THEN("Removing all scheduled tasks")
            {
                c.clear_schedules();
                REQUIRE(c.count() == 0);
            }
            AND_THEN("Removing a task that does not exist")
            {
                c.remove_schedule("Task-6");
                REQUIRE(c.count() == 5);
            }
            AND_THEN("Removing a task that does exist")
            {
                c.remove_schedule("Task-5");
                REQUIRE(c.count() == 4);
            }
        }
    }
}
