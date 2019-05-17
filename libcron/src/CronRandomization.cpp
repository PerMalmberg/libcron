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
        const std::regex split{ R"#(^\s*(.*?)\s+(.*?)\s+(.*?)\s+(.*?)\s+(.*?)\s+(.*?)\s*$)#",
                                std::regex_constants::ECMAScript };

        std::smatch all_sections;
        auto res = std::regex_match(cron_schedule.cbegin(), cron_schedule.cend(), all_sections, split);

        // Replace text with numbers
        std::string working_copy{};

        if (res)
        {
            // Merge seconds, minutes, hours and day of month back together
            working_copy += all_sections[1].str();
            working_copy += " ";
            working_copy += all_sections[2].str();
            working_copy += " ";
            working_copy += all_sections[3].str();
            working_copy += " ";
            working_copy += all_sections[4].str();
            working_copy += " ";

            // Replace month names
            auto month = all_sections[5].str();
            CronData::replace_string_name_with_numeric<libcron::Months>(month);

            working_copy += " ";
            working_copy += month;

            // Replace day names
            auto dow = all_sections[6].str();
            CronData::replace_string_name_with_numeric<libcron::DayOfWeek>(dow);

            working_copy += " ";
            working_copy += dow;
        }

        std::string final_cron_schedule{};

        // Split again on space
        res = res && std::regex_match(working_copy.cbegin(), working_copy.cend(), all_sections, split);

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
            else if (std::find(std::begin(CronData::months_with_31),
                               std::end(CronData::months_with_31),
                               month) == std::end(CronData::months_with_31))
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
