#include <catch.hpp>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <libcron/CronRandomization.h>
#include <libcron/Cron.h>
#include <iostream>

using namespace libcron;

void test(const char* const random_schedule)
{
    libcron::CronRandomization cr;
    std::unordered_map<int, std::unordered_map<int, int>> results{};

    for (int i = 0; i < 5000; ++i)
    {
        auto res = cr.parse(random_schedule);
        REQUIRE(std::get<0>(res));
        auto schedule = std::get<1>(res);

        INFO("schedule:" << schedule);
        Cron<> cron;
        REQUIRE(cron.add_schedule("validate schedule", schedule, []() {}));
    }
}

SCENARIO("Randomize all the things")
{
    const char* random_schedule = "R(0-59) R(0-59) R(0-23) R(1-31) R(1-12) ?";

    GIVEN(random_schedule)
    {
        THEN("Only valid schedules generated")
        {
            test(random_schedule);
        }
    }
}

SCENARIO("Randomize all the things with reverse ranges")
{
    const char* random_schedule = "R(45-15) R(30-0) R(18-2) R(28-15) R(8-3) ?";

    GIVEN(random_schedule)
    {
        THEN("Only valid schedules generated")
        {
            test(random_schedule);
        }
    }
}

SCENARIO("Randomize all the things - day of week")
{
    const char* random_schedule = "R(0-59) R(0-59) R(0-23) ? R(1-12) R(0-6)";

    GIVEN(random_schedule)
    {
        THEN("Only valid schedules generated")
        {
            test(random_schedule);
        }
    }
}

SCENARIO("Randomize all the things with reverse ranges - day of week")
{
    const char* random_schedule = "R(45-15) R(30-0) R(18-2) ? R(8-3) R(4-1)";

    GIVEN(random_schedule)
    {
        THEN("Only valid schedules generated")
        {
            test(random_schedule);
        }
    }
}

SCENARIO("Test readme examples")
{
    GIVEN("0 0 R(13-20) * * ?")
    {
        THEN("Valid schedule generated")
        {
            test("0 0 R(13-20) * * ?");
        }
    }

    GIVEN("0 0 0 ? * R(0-6)")
    {
        THEN("Valid schedule generated")
        {
            test("0 0 0 ? * R(0-6)");
        }
    }

    GIVEN("0 R(45-15) */12 ? * *")
    {
        THEN("Valid schedule generated")
        {
            test("0 R(45-15) */12 ? * *");
        }
    }
}
