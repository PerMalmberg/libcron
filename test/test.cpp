#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <catch.hpp>

#include <libcron/Cron.h>
#include <libcron/CronTime.h>

using namespace libcron;

template<typename T>
bool has_value_range(const std::set<T>& set, uint8_t low, uint8_t high)
{
    bool found = true;
    for (auto i = low; found && i <= high; ++i)
    {
        found &= set.find(static_cast<T>(i)) != set.end();
    }

    return found;
}

template<typename T>
bool has_any_in_range(const std::set<T>& set, uint8_t low, uint8_t high)
{
    bool found = false;
    for (auto i = low; !found && i <= high; ++i)
    {
        found |= set.find(static_cast<T>(i)) != set.end();
    }

    return found;
}

SCENARIO("Numerical inputs")
{
    GIVEN("Valid numerical inputs")
    {
        WHEN("Creating with all stars")
        {
            THEN("All parts are filled")
            {
                auto c = CronTime::create("* * * * * *");
                REQUIRE(c.is_valid());
                REQUIRE(c.seconds.size() == 60);
                REQUIRE(has_value_range(c.seconds, 0, 59));
                REQUIRE(c.minutes.size() == 60);
                REQUIRE(has_value_range(c.minutes, 0, 59));
                REQUIRE(c.hours.size() == 24);
                REQUIRE(has_value_range(c.hours, 0, 23));
                REQUIRE(c.day_of_month.size() == 31);
                REQUIRE(has_value_range(c.day_of_month, 1, 31));
                REQUIRE(c.day_of_week.size() == 7);
                REQUIRE(has_value_range(c.day_of_week, 0, 6));
            }
        }
        AND_WHEN("Using full forward range")
        {
            THEN("Ranges are correct")
            {
                auto c = CronTime::create("* 0-59 * * * *");
                REQUIRE(c.is_valid());
                REQUIRE(c.seconds.size() == 60);
                REQUIRE(c.minutes.size() == 60);
                REQUIRE(c.hours.size() == 24);
                REQUIRE(c.day_of_month.size() == 31);
                REQUIRE(c.day_of_week.size() == 7);
                REQUIRE(has_value_range(c.seconds, 0, 59));
            }
        }
        AND_WHEN("Using partial range")
        {
            THEN("Ranges are correct")
            {
                auto c = CronTime::create("* * * 20-30 * *");
                REQUIRE(c.is_valid());
                REQUIRE(c.seconds.size() == 60);
                REQUIRE(c.minutes.size() == 60);
                REQUIRE(c.hours.size() == 24);
                REQUIRE(c.day_of_month.size() == 11);
                REQUIRE(c.day_of_week.size() == 7);
                REQUIRE(has_value_range(c.day_of_month, 20, 30));
            }
        }
        AND_WHEN("Using backward range")
        {
            THEN("Number of hours are correct")
            {
                auto c = CronTime::create("* * 20-5 * * *");
                REQUIRE(c.is_valid());
                REQUIRE(c.hours.size() == 10);
                REQUIRE(c.hours.find(Hours::First) != c.hours.end());
            }
        }
        AND_WHEN("Using various ranges")
        {
            THEN("Validation succeeds")
            {
                REQUIRE(CronTime::create("0-59 * * * * *").is_valid());
                REQUIRE(CronTime::create("* 0-59 * * * *").is_valid());
                REQUIRE(CronTime::create("* * 0-23 * * *").is_valid());
                REQUIRE(CronTime::create("* * * 1-31 * *").is_valid());
                REQUIRE(CronTime::create("* * * * 1-12 *").is_valid());
                REQUIRE(CronTime::create("* * * * * 0-6").is_valid());
            }
        }
    }
    GIVEN("Invalid inputs")
    {
        WHEN("Creating items")
        {
            THEN("Validation fails")
            {
                REQUIRE_FALSE(CronTime::create("").is_valid());
                REQUIRE_FALSE(CronTime::create("-").is_valid());
                REQUIRE_FALSE(CronTime::create("* ").is_valid());
                REQUIRE_FALSE(CronTime::create("* 0-60 * * * *").is_valid());
                REQUIRE_FALSE(CronTime::create("* * 0-25 * * *").is_valid());
                REQUIRE_FALSE(CronTime::create("* * * 1-32 * *").is_valid());
                REQUIRE_FALSE(CronTime::create("* * * * 1-13 *").is_valid());
                REQUIRE_FALSE(CronTime::create("* * * * * 0-7").is_valid());
                REQUIRE_FALSE(CronTime::create("* * * 0-31 * *").is_valid());
                REQUIRE_FALSE(CronTime::create("* * * * 0-12 *").is_valid());
                REQUIRE_FALSE(CronTime::create("60 * * * * *").is_valid());
                REQUIRE_FALSE(CronTime::create("* 60 * * * *").is_valid());
                REQUIRE_FALSE(CronTime::create("* * 25 * * *").is_valid());
                REQUIRE_FALSE(CronTime::create("* * * 32 * *").is_valid());
                REQUIRE_FALSE(CronTime::create("* * * * 13 *").is_valid());
                REQUIRE_FALSE(CronTime::create("* * * * * 7").is_valid());
            }
        }
    }
}

SCENARIO("Literal input")
{
    GIVEN("Literal inputs")
    {
        WHEN("Using literal ranges")
        {
            THEN("Range is valid")
            {
                auto c = CronTime::create("* * * * JAN-MAR *");
                REQUIRE(c.is_valid());
                REQUIRE(has_value_range(c.months, 1, 3));
            }
            AND_THEN("Range is valid")
            {
                auto c = CronTime::create("* * * * * SUN-FRI");
                REQUIRE(c.is_valid());
                REQUIRE(has_value_range(c.day_of_week, 0, 5));
            }
        }
        AND_WHEN("Using both range and specific month")
        {
            THEN("Range is valid")
            {
                auto c = CronTime::create("* * * * JAN-MAR,DEC *");
                REQUIRE(c.is_valid());
                REQUIRE(has_value_range(c.months, 1, 3));
                REQUIRE_FALSE(has_any_in_range(c.months, 4, 11));
                REQUIRE(has_value_range(c.months, 12, 12));
            }
            AND_THEN("Range is valid")
            {
                auto c = CronTime::create("* * * * JAN-MAR,DEC FRI,MON,THU");
                REQUIRE(c.is_valid());
                REQUIRE(has_value_range(c.months, 1, 3));
                REQUIRE_FALSE(has_any_in_range(c.months, 4, 11));
                REQUIRE(has_value_range(c.months, 12, 12));
                REQUIRE(has_value_range(c.day_of_week, 5, 5));
                REQUIRE(has_value_range(c.day_of_week, 1, 1));
                REQUIRE(has_value_range(c.day_of_week, 4, 4));
                REQUIRE_FALSE(has_any_in_range(c.day_of_week, 0, 0));
                REQUIRE_FALSE(has_any_in_range(c.day_of_week, 2, 3));
                REQUIRE_FALSE(has_any_in_range(c.day_of_week, 6, 6));
            }
        }
        AND_WHEN("Using backward range")
        {
            THEN("Range is valid")
            {
                auto c = CronTime::create("* * * * APR-JAN *");
                REQUIRE(c.is_valid());
                REQUIRE(has_value_range(c.months, 4, 12));
                REQUIRE(has_value_range(c.months, 1, 1));
                REQUIRE_FALSE(has_any_in_range(c.months, 2, 3));
            }
            AND_THEN("Range is valid")
            {
                auto c = CronTime::create("* * * * * sat-tue,wed");
                REQUIRE(c.is_valid());
                REQUIRE(has_value_range(c.day_of_week, 6, 6)); // Has saturday
                REQUIRE(has_value_range(c.day_of_week, 0, 3)); // Has sun, mon, tue, wed
                REQUIRE_FALSE(has_any_in_range(c.day_of_week, 4, 5)); // Does not have thu or fri.
            }
        }
    }
}