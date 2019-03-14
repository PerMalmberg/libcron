#include <libcron/CronRandomization.h>

#include <regex>
#include <map>
#include <array>
#include <algorithm>
#include <iterator>
#include <libcron/TimeTypes.h>
#include <libcron/CronData.h>

namespace libcron
{
    CronRandomization::CronRandomization()
            : twister(rd())
    {
    }

    std::tuple<bool, std::string> CronRandomization::parse(const std::string& cron_schedule)
    {
        // Split on space to get each separate part, six parts expected
        std::regex split{ R"#(^\s*(.*?)\s+(.*?)\s+(.*?)\s+(.*?)\s+(.*?)\s+(.*?)\s*$)#",
                          std::regex_constants::ECMAScript };

        std::smatch all_sections;

        std::string final_cron_schedule;

        auto res = std::regex_match(cron_schedule.begin(), cron_schedule.end(), all_sections, split);

        if (res)
        {
            int selected_value = -1;
            auto second = get_random_in_range<Seconds>(all_sections[1].str(), selected_value);
            res = second.first;
            final_cron_schedule = second.second;

            auto minute = get_random_in_range<Minutes>(all_sections[2].str(), selected_value);
            res &= minute.first;
            final_cron_schedule += " " + minute.second;

            auto hour = get_random_in_range<Hours>(all_sections[3].str(), selected_value);
            res &= hour.first;
            final_cron_schedule += " " + hour.second;

            // Do Month before DayOfMonth to allow capping the allowed range.
            auto month = get_random_in_range<Months>(all_sections[5].str(), selected_value);
            res &= month.first;

            std::set<Months> month_range{};
            if (selected_value == -1)
            {
                // Month is not specific, get the range.
                CronData cr;
                res &= cr.convert_from_string_range_to_number_range<Months>(all_sections[5].str(), month_range);
            }
            else
            {
                month_range.emplace(static_cast<Months>(selected_value));
            }

            auto limits = day_limiter(month_range);

            auto day_of_month = get_random_in_range<DayOfMonth>(all_sections[4].str(),
                    selected_value,
                    limits);

            res &= day_of_month.first;
            final_cron_schedule += " " + day_of_month.second + " " + month.second;

            auto day_of_week = get_random_in_range<DayOfWeek>(all_sections[6].str(), selected_value);
            res &= day_of_week.first;
            final_cron_schedule += " " + day_of_week.second;
        }

        return { res, final_cron_schedule };
    }

    std::pair<int, int> CronRandomization::day_limiter(const std::set<Months>& months)
    {
        int max = CronData::value_of(DayOfMonth::Last);

        for (auto month : months)
        {
            if (month == Months::February)
            {
                // Limit to 29 days, possibly causing delaying schedule until next leap year.
                max = std::min(max, 29);
            }
            else if (std::find(CronData::months_with_31.begin(),
                               CronData::months_with_31.end(),
                               month) == CronData::months_with_31.end())
            {
                // Not among the months with 31 days
                max = std::min(max, 30);
            }
        }

        auto res = std::pair<int, int>{ CronData::value_of(DayOfMonth::First), max };

        return res;
    }

    int CronRandomization::cap(int value, int lower, int upper)
    {
        return std::max(std::min(value, upper), lower);
    }
}
