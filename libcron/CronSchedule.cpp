#include "CronSchedule.h"

using namespace std::chrono;
using namespace date;

namespace libcron
{

    std::chrono::system_clock::time_point
    CronSchedule::calculate_from(const std::chrono::system_clock::time_point& from)
    {
        auto curr = from;

        year_month_day ymd = date::floor<days>(curr);

        // Add months until one of the allowed days are found, or stay at the current one.
        while (data.get_months().find(static_cast<Months>(unsigned(ymd.month()))) == data.get_months().end())
        {
            curr += months{1};
            ymd = date::floor<days>(curr);
        };


        // If all days are allowed, then the 'day of week' takes precedence, which also means that
        // day of week only is ignored when specific days of months are specified.
        if (data.get_day_of_month().size() != CronData::value_of(DayOfMonth::Last))
        {
            // Add days until one of the allowed days are found, or stay at the current one.
            while (data.get_day_of_month().find(static_cast<DayOfMonth>(unsigned(ymd.day()))) ==
                   data.get_day_of_month().end())
            {
                curr += days{1};
                ymd = date::floor<days>(curr);
            };
        }
        else
        {
            //Add days until the current weekday is one of the allowed weekdays
            year_month_weekday ymw = date::floor<days>(curr);

            while (data.get_day_of_week().find(static_cast<DayOfWeek>(unsigned(ymw.weekday()))) ==
                   data.get_day_of_week().end())
            {
                curr += days{1};
                ymw = date::floor<days>(curr);
            };
        }

        // 'curr' now represents the next year, month and day matching the expression, with a time of 0:0:0.

        // Re-add the hours, minutes and seconds to 'curr'
        auto date_time = to_calendar_time(from);
        curr += hours{date_time.hour};
        curr += minutes{date_time.min};
        curr += seconds{date_time.sec};

        auto t = to_calendar_time(from);

        return curr;
    }
}