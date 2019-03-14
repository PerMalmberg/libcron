#include <catch.hpp>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <libcron/CronRandomization.h>
#include <libcron/Cron.h>
#include <iostream>

using namespace libcron;

SCENARIO("Randomize all the things")
{
    const char* full_random = "R(0-59) R(0-59) R(0-23) R(1-31) R(1-12) ?";
    Cron<> cron;

    GIVEN(full_random)
    {
        THEN("Only valid schedules generated")
        {
            libcron::CronRandomization cr;
            std::unordered_map<int, std::unordered_map<int, int>> results{};

            for (int i = 0; i < 1000; ++i)
            {
                auto res = cr.parse(full_random);
                REQUIRE(std::get<0>(res));
                auto schedule = std::get<1>(res);

                auto start = schedule.begin();
                auto space = std::find(schedule.begin(), schedule.end(), ' ');

                for (int section = 0; start != schedule.end() && section < 5; ++section)
                {
                    auto& map = results[section];
                    auto s = std::string{start, space};
                    auto value = std::stoi(s);
                    map[value]++;
                    start = space + 1;
                    space = std::find(start, schedule.end(), ' ');
                }

                REQUIRE(results.size() == 5);
                INFO("schedule:" << schedule);
                REQUIRE(cron.add_schedule("validate schedule", schedule, []() {}));
            }
        }
    }
}

SCENARIO("Randomize all the things with reverse ranges")
{
    // Only generate DayOfMonth up to 28 to prevent failing tests where the month doesn't have more days.
    const char* full_random = "R(45-15) R(30-0) R(18-2) R(28-15) 2 ?";
    Cron<> cron;

    GIVEN(full_random)
    {
        THEN("Only valid schedules generated")
        {
            libcron::CronRandomization cr;
            std::unordered_map<int, std::unordered_map<int, int>> results{};

            for (int i = 0; i < 1000; ++i)
            {
                auto res = cr.parse(full_random);
                REQUIRE(std::get<0>(res));
                auto schedule = std::get<1>(res);

                auto start = schedule.begin();
                auto space = std::find(schedule.begin(), schedule.end(), ' ');

                for (int section = 0; start != schedule.end() && section < 5; ++section)
                {
                    auto& map = results[section];
                    auto s = std::string{start, space};
                    auto value = std::stoi(s);
                    map[value]++;
                    start = space + 1;
                    space = std::find(start, schedule.end(), ' ');
                }

                REQUIRE(results.size() == 5);
                INFO("schedule:" << schedule);
                REQUIRE(cron.add_schedule("validate schedule", schedule, []() {}));
            }
        }
    }
}
