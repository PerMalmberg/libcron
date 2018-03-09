#include "CronSchedule.h"

using namespace std::chrono;
using namespace date;

namespace libcron
{

    std::chrono::system_clock::time_point
    CronSchedule::calculate_from(const std::chrono::system_clock::time_point& from)
    {
        auto curr = from;

        bool done = false;
        while (!done)
        {
            year_month_day ymd = date::floor<days>(curr);

            // Add months until one of the allowed days are found, or stay at the current one.
            if (data.get_months().find(static_cast<Months>(unsigned(ymd.month()))) == data.get_months().end())
            {
                curr += months{1};
                ymd = date::floor<days>(curr);
                continue;
            }

            // If all days are allowed, then the 'day of week' takes precedence, which also means that
            // day of week only is ignored when specific days of months are specified.
            if (data.get_day_of_month().size() != CronData::value_of(DayOfMonth::Last))
            {
                // Add days until one of the allowed days are found, or stay at the current one.
                if(data.get_day_of_month().find(static_cast<DayOfMonth>(unsigned(ymd.day()))) ==
                       data.get_day_of_month().end())
                {
                    curr += days{1};
                    ymd = date::floor<days>(curr);
                    continue;
                };
            }
            else
            {
                //Add days until the current weekday is one of the allowed weekdays
                year_month_weekday ymw = date::floor<days>(curr);

                if (data.get_day_of_week().find(static_cast<DayOfWeek>(unsigned(ymw.weekday()))) ==
                       data.get_day_of_week().end())
                {
                    curr += days{1};
                    ymw = date::floor<days>(curr);
                    continue;
                };
            }

            // Add hours until the current hour is one of the allowed
            auto date_time = to_calendar_time(curr);
            if (data.get_hours().find(static_cast<Hours>(date_time.hour)) == data.get_hours().end())
            {
                curr += hours{1};
                continue;
            }
            else if (data.get_minutes().find(static_cast<Minutes >(date_time.min)) == data.get_minutes().end())
            {
                curr += minutes{1};
                continue;
            }
            else if (data.get_seconds().find(static_cast<Seconds>(date_time.sec)) == data.get_seconds().end())
            {
                curr += seconds{1};
                continue;
            }
            else if( curr <= from )
            {
                curr += seconds{1};
            }
            else
            {
                done = true;
            }
        }


        return curr;
    }
}