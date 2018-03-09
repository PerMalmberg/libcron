#include <catch.hpp>
#include <chrono>
#include <date.h>
#include <libcron/Cron.h>
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

bool test(const std::string& schedule, system_clock::time_point from, system_clock::time_point expected_next)
{
    auto c = CronData::create(schedule);
    bool res = c.is_valid();
    if (res)
    {
        CronSchedule sched(c);
        auto run_time = sched.calculate_from(from);
        res &= expected_next == run_time;

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
    REQUIRE(test("0 0 * * * *", DT(2010_y / 1 / 1), DT(2010_y / 1 / 1, hours{0})));
    REQUIRE(test("0 0 * * * *", DT(2010_y / 1 / 1, hours{0}, minutes{0}, seconds{1}), DT(2010_y / 1 / 1, hours{1})));
    REQUIRE(test("0 0 * * * *", DT(2010_y / 1 / 1, hours{5}), DT(2010_y / 1 / 1, hours{5})));
    REQUIRE(test("0 0 * * * *", DT(2010_y / 1 / 1, hours{5}, minutes{1}), DT(2010_y / 1 / 1, hours{6})));
    REQUIRE(test("0 0 * * * *", DT(2017_y / 12 / 31, hours{23}, minutes{59}, seconds{58}), DT(2018_y / 1 / 1, hours{0})));
    REQUIRE(test("0 0 10 * * *", DT(2017_y / 12 / 31, hours{9}, minutes{59}, seconds{58}), DT(2017_y / 12 / 31, hours{10})));
    REQUIRE(test("0 0 10 * * *", DT(2017_y / 12 / 31, hours{23}, minutes{59}, seconds{58}), DT(2018_y / 1 / 1, hours{10})));
    REQUIRE(test("0 0 10 * FEB *", DT(2017_y / 12 / 31, hours{23}, minutes{59}, seconds{58}), DT(2018_y / 2 / 1, hours{10})));
    REQUIRE(test("0 0 10 25 FEB *", DT(2017_y / 12 / 31, hours{23}, minutes{59}, seconds{58}), DT(2018_y / 2 / 25, hours{10})));
    REQUIRE(test("0 0 10 * FEB 1", DT(2017_y / 12 / 31, hours{23}, minutes{59}, seconds{58}), DT(year_month_day{2018_y/2/mon[1]}, hours{10})));
    REQUIRE(test("0 0 10 * FEB 6", DT(2017_y / 12 / 31, hours{23}, minutes{59}, seconds{58}), DT(year_month_day{2018_y/2/sat[1]}, hours{10})));
    REQUIRE(test("* * * 10-12 NOV *", DT(2018_y / 11 / 11, hours{10}, minutes{11}, seconds{12}), DT(year_month_day{2018_y/11/11}, hours{10}, minutes{11}, seconds{12})));
}

SCENARIO("Leap year")
{
    REQUIRE(test("0 0 * 29 FEB *", DT(2018_y / 1 / 1), DT(2020_y / 2 / 29)));
}

SCENARIO("Date that does not exist")
{
    //REQUIRE_FALSE(test("0 0 * 30 FEB *", DT(2018_y / 1 / 1), DT(2020_y / 2 / 29)));
}