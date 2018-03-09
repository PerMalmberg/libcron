#include "CronData.h"

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
                               { return !std::isdigit(c); }) == s.end();
    }

    bool CronData::is_between(int32_t value, int32_t low_limit, int32_t high_limt)
    {
        return value >= low_limit && value <= high_limt;
    }
}