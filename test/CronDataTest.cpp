#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <catch.hpp>
#include <date/date.h>
#include <libcron/include/libcron/Cron.h>
#include <libcron/include/libcron/CronData.h>

using namespace libcron;
using namespace date;
using namespace std::chrono;

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

SCENARIO("Numerical inputs")
{
    GIVEN("Valid numerical inputs")
    {
        WHEN("Creating with all stars")
        {
            THEN("All parts are filled")
            {
                auto c = CronData::create("* * * * * ?");
                REQUIRE(c.is_valid());
                REQUIRE(c.get_seconds().size() == 60);
                REQUIRE(has_value_range(c.get_seconds(), 0, 59));
                REQUIRE(c.get_minutes().size() == 60);
                REQUIRE(has_value_range(c.get_minutes(), 0, 59));
                REQUIRE(c.get_hours().size() == 24);
                REQUIRE(has_value_range(c.get_hours(), 0, 23));
                REQUIRE(c.get_day_of_month().size() == 31);
                REQUIRE(has_value_range(c.get_day_of_month(), 1, 31));
                REQUIRE(c.get_day_of_week().size() == 7);
                REQUIRE(has_value_range(c.get_day_of_week(), 0, 6));
            }
        }
        AND_WHEN("Using full forward range")
        {
            THEN("Ranges are correct")
            {
                auto c = CronData::create("* 0-59 * * * ?");
                REQUIRE(c.is_valid());
                REQUIRE(c.get_seconds().size() == 60);
                REQUIRE(c.get_minutes().size() == 60);
                REQUIRE(c.get_hours().size() == 24);
                REQUIRE(c.get_day_of_month().size() == 31);
                REQUIRE(c.get_day_of_week().size() == 7);
                REQUIRE(has_value_range(c.get_seconds(), 0, 59));
            }
        }
        AND_WHEN("Using partial range")
        {
            THEN("Ranges are correct")
            {
                auto c = CronData::create("* * * 20-30 * ?");
                REQUIRE(c.is_valid());
                REQUIRE(c.get_seconds().size() == 60);
                REQUIRE(c.get_minutes().size() == 60);
                REQUIRE(c.get_hours().size() == 24);
                REQUIRE(c.get_day_of_month().size() == 11);
                REQUIRE(c.get_day_of_week().size() == 7);
                REQUIRE(has_value_range(c.get_day_of_month(), 20, 30));
            }
        }
        AND_WHEN("Using backward range")
        {
            THEN("Number of hours are correct")
            {
                auto c = CronData::create("* * 20-5 * * ?");
                REQUIRE(c.is_valid());
                REQUIRE(c.get_hours().size() == 10);
                REQUIRE(c.get_hours().find(Hours::First) != c.get_hours().end());
            }
        }
        AND_WHEN("Using various ranges")
        {
            THEN("Validation succeeds")
            {
                REQUIRE(CronData::create("0-59 * * * * ?").is_valid());
                REQUIRE(CronData::create("* 0-59 * * * ?").is_valid());
                REQUIRE(CronData::create("* * 0-23 * * ?").is_valid());
                REQUIRE(CronData::create("* * * 1-31 * ?").is_valid());
                REQUIRE(CronData::create("* * * * 1-12 ?").is_valid());
                REQUIRE(CronData::create("* * * ? * 0-6").is_valid());
            }
        }
    }
    GIVEN("Invalid inputs")
    {
        WHEN("Creating items")
        {
            THEN("Validation fails")
            {
                REQUIRE_FALSE(CronData::create("").is_valid());
                REQUIRE_FALSE(CronData::create("-").is_valid());
                REQUIRE_FALSE(CronData::create("* ").is_valid());
                REQUIRE_FALSE(CronData::create("* 0-60 * * * ?").is_valid());
                REQUIRE_FALSE(CronData::create("* * 0-25 * * ?").is_valid());
                REQUIRE_FALSE(CronData::create("* * * 1-32 * ?").is_valid());
                REQUIRE_FALSE(CronData::create("* * * * 1-13 ?").is_valid());
                REQUIRE_FALSE(CronData::create("* * * * * 0-7").is_valid());
                REQUIRE_FALSE(CronData::create("* * * 0-31 * ?").is_valid());
                REQUIRE_FALSE(CronData::create("* * * * 0-12 ?").is_valid());
                REQUIRE_FALSE(CronData::create("60 * * * * ?").is_valid());
                REQUIRE_FALSE(CronData::create("* 60 * * * ?").is_valid());
                REQUIRE_FALSE(CronData::create("* * 25 * * ?").is_valid());
                REQUIRE_FALSE(CronData::create("* * * 32 * ?").is_valid());
                REQUIRE_FALSE(CronData::create("* * * * 13 ?").is_valid());
                REQUIRE_FALSE(CronData::create("* * * ? * 7").is_valid());
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
                auto c = CronData::create("* * * * JAN-MAR ?");
                REQUIRE(c.is_valid());
                REQUIRE(has_value_range(c.get_months(), 1, 3));
            }
            AND_THEN("Range is valid")
            {
                auto c = CronData::create("* * * ? * SUN-FRI");
                REQUIRE(c.is_valid());
                REQUIRE(has_value_range(c.get_day_of_week(), 0, 5));
            }
        }
        AND_WHEN("Using both range and specific month")
        {
            THEN("Range is valid")
            {
                auto c = CronData::create("* * * * JAN-MAR,DEC ?");
                REQUIRE(c.is_valid());
                REQUIRE(has_value_range(c.get_months(), 1, 3));
                REQUIRE_FALSE(CronData::has_any_in_range(c.get_months(), 4, 11));
                REQUIRE(has_value_range(c.get_months(), 12, 12));
            }
            AND_THEN("Range is valid")
            {
                auto c = CronData::create("* * * ? JAN-MAR,DEC FRI,MON,THU");
                REQUIRE(c.is_valid());
                REQUIRE(has_value_range(c.get_months(), 1, 3));
                REQUIRE_FALSE(CronData::has_any_in_range(c.get_months(), 4, 11));
                REQUIRE(has_value_range(c.get_months(), 12, 12));
                REQUIRE(has_value_range(c.get_day_of_week(), 5, 5));
                REQUIRE(has_value_range(c.get_day_of_week(), 1, 1));
                REQUIRE(has_value_range(c.get_day_of_week(), 4, 4));
                REQUIRE_FALSE(CronData::has_any_in_range(c.get_day_of_week(), 0, 0));
                REQUIRE_FALSE(CronData::has_any_in_range(c.get_day_of_week(), 2, 3));
                REQUIRE_FALSE(CronData::has_any_in_range(c.get_day_of_week(), 6, 6));
            }
        }
        AND_WHEN("Using backward range")
        {
            THEN("Range is valid")
            {
                auto c = CronData::create("* * * ? APR-JAN *");
                REQUIRE(c.is_valid());
                REQUIRE(has_value_range(c.get_months(), 4, 12));
                REQUIRE(has_value_range(c.get_months(), 1, 1));
                REQUIRE_FALSE(CronData::has_any_in_range(c.get_months(), 2, 3));
            }
            AND_THEN("Range is valid")
            {
                auto c = CronData::create("* * * ? * sat-tue,wed");
                REQUIRE(c.is_valid());
                REQUIRE(has_value_range(c.get_day_of_week(), 6, 6)); // Has saturday
                REQUIRE(has_value_range(c.get_day_of_week(), 0, 3)); // Has sun, mon, tue, wed
                REQUIRE_FALSE(CronData::has_any_in_range(c.get_day_of_week(), 4, 5)); // Does not have thu or fri.
            }
        }
    }
}

SCENARIO("Using step syntax")
{
    GIVEN("Step inputs")
    {
        WHEN("Using literal ranges")
        {
            THEN("Range is valid")
            {
                auto c = CronData::create("* * * * JAN/2 ?");
                REQUIRE(c.is_valid());
                REQUIRE(has_value_range(c.get_months(), 1, 1));
                REQUIRE(has_value_range(c.get_months(), 3, 3));
                REQUIRE(has_value_range(c.get_months(), 5, 5));
                REQUIRE(has_value_range(c.get_months(), 7, 7));
                REQUIRE(has_value_range(c.get_months(), 9, 9));
                REQUIRE(has_value_range(c.get_months(), 11, 11));
                REQUIRE_FALSE(CronData::has_any_in_range(c.get_months(), 2, 2));
                REQUIRE_FALSE(CronData::has_any_in_range(c.get_months(), 4, 4));
                REQUIRE_FALSE(CronData::has_any_in_range(c.get_months(), 6, 6));
                REQUIRE_FALSE(CronData::has_any_in_range(c.get_months(), 8, 8));
                REQUIRE_FALSE(CronData::has_any_in_range(c.get_months(), 10, 10));
                REQUIRE_FALSE(CronData::has_any_in_range(c.get_months(), 12, 12));
            }

        }
    }
}

SCENARIO("Dates that does not exist")
{
    REQUIRE_FALSE(CronData::create("0 0 * 30 FEB *").is_valid());
    REQUIRE_FALSE(CronData::create("0 0 * 31 APR *").is_valid());
}

SCENARIO("Date that exist in one of the months")
{
    REQUIRE(CronData::create("0 0 * 31 APR,MAY ?").is_valid());
}

SCENARIO("Replacing text with numbers")
{
    {
        std::string s = "SUN-TUE";
        REQUIRE(CronData::replace_string_name_with_numeric<libcron::DayOfWeek>(s) == "0-2");
    }

    {
        std::string s = "JAN-DEC";
        REQUIRE(CronData::replace_string_name_with_numeric<libcron::Months>(s) == "1-12");
    }
}