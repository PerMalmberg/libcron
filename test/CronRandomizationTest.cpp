#include <catch.hpp>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <libcron/CronRandomization.h>
#include <libcron/Cron.h>
#include <iostream>

using namespace libcron;
const auto EXPECT_FAILURE = true;

void test(const char* const random_schedule, bool expect_failure = false)
{
    libcron::CronRandomization cr;

    for (int i = 0; i < 5000; ++i)
    {
        auto res = cr.parse(random_schedule);
        auto schedule = std::get<1>(res);

        Cron<> cron;

        if(expect_failure)
        {
            // Parsing of random might succeed, but it yields an invalid schedule.
            auto r = std::get<0>(res) && cron.add_schedule("validate schedule", schedule, [](auto&) {});
            REQUIRE_FALSE(r);
        }
        else
        {
            REQUIRE(std::get<0>(res));
            REQUIRE(cron.add_schedule("validate schedule", schedule, [](auto&) {}));

        }
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

SCENARIO("Randomization using text versions of days and months")
{
    GIVEN("0 0 0 ? * R(TUE-FRI)")
    {
        THEN("Valid schedule generated")
        {
           test("0 0 0 ? * R(TUE-FRI)");
        }
    }

    GIVEN("Valid schedule")
    {
        THEN("Valid schedule generated")
        {
            test("0 0 0 ? R(JAN-DEC) R(MON-FRI)");
        }
        AND_WHEN("Given 0 0 0 ? R(DEC-MAR) R(SAT-SUN)")
        {
            THEN("Valid schedule generated")
            {
                test("0 0 0 ? R(DEC-MAR) R(SAT-SUN)");
            }
        }
        AND_THEN("Given 0 0 0 ? R(JAN-FEB) *")
        {
            THEN("Valid schedule generated")
            {
                test("0 0 0 ? R(JAN-FEB) *");
            }
        }
        AND_THEN("Given 0 0 0 ? R(OCT-OCT) *")
        {
            THEN("Valid schedule generated")
            {
                test("0 0 0 ? R(OCT-OCT) *");
            }
        }
    }

    GIVEN("Invalid schedule")
    {
        THEN("No schedule generated")
        {
            // Day of month specified - not allowed with day of week
            test("0 0 0 1 R(JAN-DEC) R(MON-SUN)", EXPECT_FAILURE);
        }
        AND_THEN("No schedule generated")
        {
            // Invalid range
            test("0 0 0 ? R(JAN) *", EXPECT_FAILURE);
        }
        AND_THEN("No schedule generated")
        {
            // Days in month field
            test("0 0 0 ? R(MON-TUE) *", EXPECT_FAILURE);
        }
        AND_THEN("No schedule generated")
        {
            // Month in day field
            test("0 0 0 ? * R(JAN-JUN)", EXPECT_FAILURE);
        }

    }
}
