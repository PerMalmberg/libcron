#include <date.h>
#include "CronData.h"

using namespace date;

namespace libcron
{

    CronData CronData::create(const std::string& cron_expression)
    {
        CronData c;
        c.parse(cron_expression);

        return std::move(c);
    }

    CronData::CronData()
            : month_names({"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"}),
              day_names({"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"})
    {
    }

    void CronData::parse(const std::string& cron_expression)
    {
        // First, split on white-space. We expect six parts.
        std::regex split{R"#(^\s*(.*?)\s+(.*?)\s+(.*?)\s+(.*?)\s+(.*?)\s+(.*?)\s*$)#",
                         std::regex_constants::ECMAScript};

        std::smatch match;

        if (std::regex_match(cron_expression.begin(), cron_expression.end(), match, split))
        {
            valid = validate_numeric<Seconds>(match[1], seconds);
            valid &= validate_numeric<Minutes>(match[2], minutes);
            valid &= validate_numeric<Hours>(match[3], hours);
            valid &= validate_numeric<DayOfMonth>(match[4], day_of_month);
            valid &= validate_literal<Months>(match[5], months, month_names);
            valid &= validate_literal<DayOfWeek>(match[6], day_of_week, day_names);
            valid &= validate_date_vs_months();
        }
    }

    std::vector<std::string> CronData::split(const std::string& s, char token)
    {
        std::vector<std::string> res;

        std::string r = "[";
        r += token;
        r += "]";
        std::regex splitter{r, std::regex_constants::ECMAScript};

        std::copy(std::sregex_token_iterator(s.begin(), s.end(), splitter, -1),
                  std::sregex_token_iterator(),
                  std::back_inserter(res));


        return res;
    }

    bool CronData::is_number(const std::string& s)
    {
        // Find any character that isn't a number.
        return !s.empty()
               && std::find_if(s.begin(), s.end(),
                               [](char c)
                               {
                                   return !std::isdigit(c);
                               }) == s.end();
    }

    bool CronData::is_between(int32_t value, int32_t low_limit, int32_t high_limt)
    {
        return value >= low_limit && value <= high_limt;
    }

    bool CronData::validate_date_vs_months() const
    {
        bool res = true;

        // Verify that the available dates are possible based on the given months
        if (months.size() == 1 && months.find(static_cast<Months>(2)) != months.end())
        {
            // Only february allowed, make sure that the allowed date(s) includes 29 and below.
            res = has_any_in_range(day_of_month, 1, 29);
        }

        if(res)
        {
            // Make sure that if the days contains only 31, at least one month allows that date.
            if(day_of_month.size() == 1 && day_of_month.find(DayOfMonth::Last) != day_of_month.end())
            {
                std::vector<int32_t> months_with_31;
                for (int32_t i = 1; i <= 12; ++i)
                {
                    auto ymd = 2018_y / i / date::last;
                    if (unsigned(ymd.day()) == 31)
                    {
                        months_with_31.push_back(i);
                    }
                }

                res = false;
                for(size_t i = 0; !res && i < months_with_31.size(); ++i)
                {
                    res = months.find(static_cast<Months>(months_with_31[i])) != months.end();
                }
            }
        }


        return res;
    }
}