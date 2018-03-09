#pragma once

#include "CronData.h"
#include <chrono>
#include "externals/date/date.h"
#include "DateTime.h"

namespace libcron
{
    class CronSchedule
    {
        public:
            explicit CronSchedule(CronData data)
                    : data(std::move(data))
            {
            }

            std::chrono::system_clock::time_point calculate_from(const std::chrono::system_clock::time_point& from);

            // https://github.com/HowardHinnant/date/wiki/Examples-and-Recipes#obtaining-ymd-hms-components-from-a-time_point
            static DateTime to_calendar_time(std::chrono::system_clock::time_point time)
            {
                auto daypoint = date::floor<date::days>(time);
                auto ymd = date::year_month_day(daypoint);   // calendar date
                auto tod = date::make_time(time - daypoint); // Yields time_of_day type

                // Obtain individual components as integers
                DateTime dt{
                        int(ymd.year()),
                        unsigned(ymd.month()),
                        unsigned(ymd.day()),
                        static_cast<uint8_t>(tod.hours().count()),
                        static_cast<uint8_t>(tod.minutes().count()),
                        static_cast<uint8_t>(tod.seconds().count())};

                return dt;
            }


        private:
            CronData data;
    };

}