#include <catch.hpp>
#include <chrono>
#include <date/date.h>
#include <libcron/include/libcron/Cron.h>
#include <iostream>

using namespace libcron;
using namespace date;
using namespace std::chrono;

system_clock::time_point DT(year_month_day ymd, hours h = hours{0}, minutes m = minutes{0}, seconds s = seconds{0})
{
    sys_days t = ymd;
    auto sum = t + h + m + s;
    return sum;
}

bool test(const std::string& schedule, system_clock::time_point from,
          const std::vector<system_clock::time_point>& expected_next)
{
    auto c = CronData::create(schedule);
    bool res = c.is_valid();
    if (res)
    {
        CronSchedule sched(c);

        auto curr_from = from;

        for (size_t i = 0; res && i < expected_next.size(); ++i)
        {
            auto result = sched.calculate_from(curr_from);
            auto calculated = std::get<1>(result);

            res = std::get<0>(result) && calculated == expected_next[i];

            if (res)
            {
                // Add a second to the time so that we move on to the next expected time
                // and don't get locked on the current one.
                curr_from = expected_next[i] + seconds{1};
            }
            else
            {
                std::cout
                        << "From:       " << curr_from << "\n"
                        << "Expected:   " << expected_next[i] << "\n"
                        << "Calculated: " << calculated;
            }
        }
    }

    return res;
}

bool test(const std::string& schedule, system_clock::time_point from, system_clock::time_point expected_next)
{
    auto c = CronData::create(schedule);
    bool res = c.is_valid();
    if (res)
    {
        CronSchedule sched(c);
        auto result = sched.calculate_from(from);
        auto run_time = std::get<1>(result);
        res &= std::get<0>(result) && expected_next == run_time;

        if (!res)
        {
            std::cout
                    << "From:       " << from << "\n"
                    << "Expected:   " << expected_next << "\n"
                    << "Calculated: " << run_time;
        }
    }

    return res;
}

SCENARIO("Calculating next runtime")
{
    REQUIRE(test("0 0 * * * ?", DT(2010_y / 1 / 1), DT(2010_y / 1 / 1, hours{0})));
    REQUIRE(test("0 0 * * * ?", DT(2010_y / 1 / 1, hours{0}, minutes{0}, seconds{1}), DT(2010_y / 1 / 1, hours{1})));
    REQUIRE(test("0 0 * * * ?", DT(2010_y / 1 / 1, hours{5}), DT(2010_y / 1 / 1, hours{5})));
    REQUIRE(test("0 0 * * * ?", DT(2010_y / 1 / 1, hours{5}, minutes{1}), DT(2010_y / 1 / 1, hours{6})));
    REQUIRE(test("0 0 * * * ?", DT(2017_y / 12 / 31, hours{23}, minutes{59}, seconds{58}),
                 DT(2018_y / 1 / 1, hours{0})));
    REQUIRE(test("0 0 10 * * ?", DT(2017_y / 12 / 31, hours{9}, minutes{59}, seconds{58}),
                 DT(2017_y / 12 / 31, hours{10})));
    REQUIRE(test("0 0 10 * * ?", DT(2017_y / 12 / 31, hours{23}, minutes{59}, seconds{58}),
                 DT(2018_y / 1 / 1, hours{10})));
    REQUIRE(test("0 0 10 ? FEB *", DT(2017_y / 12 / 31, hours{23}, minutes{59}, seconds{58}),
                 DT(2018_y / 2 / 1, hours{10})));
    REQUIRE(test("0 0 10 25 FEB ?", DT(2017_y / 12 / 31, hours{23}, minutes{59}, seconds{58}),
                 DT(2018_y / 2 / 25, hours{10})));
    REQUIRE(test("0 0 10 ? FEB 1", DT(2017_y / 12 / 31, hours{23}, minutes{59}, seconds{58}),
                 DT(year_month_day{2018_y / 2 / mon[1]}, hours{10})));
    REQUIRE(test("0 0 10 ? FEB 6", DT(2017_y / 12 / 31, hours{23}, minutes{59}, seconds{58}),
                 DT(year_month_day{2018_y / 2 / sat[1]}, hours{10})));
    REQUIRE(test("* * ? 10-12 NOV ?", DT(2018_y / 11 / 11, hours{10}, minutes{11}, seconds{12}),
                 DT(year_month_day{2018_y / 11 / 11}, hours{10}, minutes{11}, seconds{12})));
    REQUIRE(test("0 0 * 31 APR,MAY ?", DT(2017_y / 6 / 1), DT(2018_y / may / 31)));
}

SCENARIO("Leap year")
{
    REQUIRE(test("0 0 * 29 FEB *", DT(2015_y / 1 / 1), DT(2016_y / 2 / 29)));
    REQUIRE(test("0 0 * 29 FEB ?", DT(2018_y / 1 / 1), DT(2020_y / 2 / 29)));
    REQUIRE(test("0 0 * 29 FEB ?", DT(2020_y / 2 / 29, hours{15}, minutes{13}, seconds{13}),
                 DT(2020_y / 2 / 29, hours{16})));
}

SCENARIO("Multiple calculations")
{
    WHEN("Every 15 minutes, every 2nd hour")
    {
        REQUIRE(test("0 0/15 0/2 * * ?", DT(2018_y / 1 / 1, hours{13}, minutes{14}, seconds{59}),
                     {DT(2018_y / 1 / 1, hours{14}, minutes{00}),
                      DT(2018_y / 1 / 1, hours{14}, minutes{15}),
                      DT(2018_y / 1 / 1, hours{14}, minutes{30}),
                      DT(2018_y / 1 / 1, hours{14}, minutes{45}),
                      DT(2018_y / 1 / 1, hours{16}, minutes{00}),
                      DT(2018_y / 1 / 1, hours{16}, minutes{15})}));
    }

    WHEN("Every top of the hour, every 12th hour, during 12 and 13:th July")
    {
        REQUIRE(test("0 0 0/12 12-13 JUL ?", DT(2018_y / 1 / 1),
                     {DT(2018_y / 7 / 12, hours{0}),
                      DT(2018_y / 7 / 12, hours{12}),
                      DT(2018_y / 7 / 13, hours{0}),
                      DT(2018_y / 7 / 13, hours{12}),
                      DT(2019_y / 7 / 12, hours{0}),
                      DT(2019_y / 7 / 12, hours{12})}));
    }

    WHEN("Every first of the month, 15h, every second month, 22m")
    {
        REQUIRE(test("0 22 15 1 * ?", DT(2018_y / 1 / 1),
                     {DT(2018_y / 1 / 1, hours{15}, minutes{22}),
                      DT(2018_y / 2 / 1, hours{15}, minutes{22}),
                      DT(2018_y / 3 / 1, hours{15}, minutes{22}),
                      DT(2018_y / 4 / 1, hours{15}, minutes{22}),
                      DT(2018_y / 5 / 1, hours{15}, minutes{22}),
                      DT(2018_y / 6 / 1, hours{15}, minutes{22}),
                      DT(2018_y / 7 / 1, hours{15}, minutes{22}),
                      DT(2018_y / 8 / 1, hours{15}, minutes{22}),
                      DT(2018_y / 9 / 1, hours{15}, minutes{22}),
                      DT(2018_y / 10 / 1, hours{15}, minutes{22}),
                      DT(2018_y / 11 / 1, hours{15}, minutes{22}),
                      DT(2018_y / 12 / 1, hours{15}, minutes{22}),
                      DT(2019_y / 1 / 1, hours{15}, minutes{22})}));
    }

    WHEN("“At minute 0 past hour 0 and 12 on day-of-month 1 in every 2nd month")
    {
        REQUIRE(test("0 0 0,12 1 */2 ?", DT(2018_y / 3 / 10, hours{16}, minutes{51}), DT(2018_y / 5 / 1)));
    }

    WHEN("“At 00:05 in August")
    {
        REQUIRE(test("0 5 0 * 8 ?", DT(2018_y / 3 / 10, hours{16}, minutes{51}),
                     {DT(2018_y / 8 / 1, hours{0}, minutes{5}),
                      DT(2018_y / 8 / 2, hours{0}, minutes{5})}));
    }

    WHEN("At 22:00 on every day-of-week from Monday through Friday")
    {
        REQUIRE(test("0 0 22 ? * 1-5", DT(2021_y / 12 / 15, hours{16}, minutes{51}),
                     {DT(2021_y / 12 / 15, hours{22}),
                      DT(2021_y / 12 / 16, hours{22}),
                      DT(2021_y / 12 / 17, hours{22}),
                             // 18-19 are weekend
                      DT(2021_y / 12 / 20, hours{22}),
                      DT(2021_y / 12 / 21, hours{22})}));
    }
}

SCENARIO("Examples from README.md")
{
    REQUIRE(test("* * * * * ?", DT(2018_y / 03 / 1, hours{12}, minutes{13}, seconds{45}),
                 {
                         DT(2018_y / 03 / 1, hours{12}, minutes{13}, seconds{45}),
                         DT(2018_y / 03 / 1, hours{12}, minutes{13}, seconds{46}),
                         DT(2018_y / 03 / 1, hours{12}, minutes{13}, seconds{47}),
                         DT(2018_y / 03 / 1, hours{12}, minutes{13}, seconds{48})
                 }));

    REQUIRE(test("0 * * * * ?", DT(2018_y / 03 / 1, hours{ 12 }, minutes{ 0 }, seconds{ 10 }),
        {
                DT(2018_y / 03 / 1, hours{12}, minutes{1}, seconds{0}),
                DT(2018_y / 03 / 1, hours{12}, minutes{2}, seconds{0}),
                DT(2018_y / 03 / 1, hours{12}, minutes{3}, seconds{0}),
                DT(2018_y / 03 / 1, hours{12}, minutes{4}, seconds{0})
        }));


    REQUIRE(test("0 0 12 * * MON-FRI", DT(2018_y / 03 / 10, hours{12}, minutes{13}, seconds{45}),
                 {
                         DT(2018_y / 03 / 12, hours{12}),
                         DT(2018_y / 03 / 13, hours{12}),
                         DT(2018_y / 03 / 14, hours{12}),
                         DT(2018_y / 03 / 15, hours{12}),
                         DT(2018_y / 03 / 16, hours{12}),
                         DT(2018_y / 03 / 19, hours{12})
                 }));

    REQUIRE(test("0 0 12 1/2 * ?", DT(2018_y / 01 / 2, hours{12}, minutes{13}, seconds{45}),
                 {
                         DT(2018_y / 1 / 3, hours{12}),
                         DT(2018_y / 1 / 5, hours{12}),
                         DT(2018_y / 1 / 7, hours{12})
                 }));

    REQUIRE(test("0 0 */12 ? * *", DT(2018_y / 8 / 15, hours{13}, minutes{13}, seconds{45}),
                 {
                         DT(2018_y / 8 / 16, hours{0}),
                         DT(2018_y / 8 / 16, hours{12}),
                         DT(2018_y / 8 / 17, hours{0})
                 }));
}

SCENARIO("Unable to calculate time point")
{
    REQUIRE_FALSE(test( "0 0 * 31 FEB *", DT(2021_y / 1 / 1), DT(2022_y / 1 / 1)));
}
