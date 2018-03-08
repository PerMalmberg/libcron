#include "CronTime.h"

namespace libcron
{

    CronTime CronTime::create(const std::string& cron_expression)
    {
        CronTime c;
        c.parse(cron_expression);

        return std::move(c);
    }

    CronTime::CronTime()
            : month_names({"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"}),
              day_names({"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"})
    {
    }

    void CronTime::parse(const std::string& cron_expression)
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
            valid &= validate_numeric<Months>(match[5], months)
                     || validate_literal<Months>(match[5], months, month_names, 1);
            valid &= validate_numeric<DayOfWeek>(match[6], day_of_week)
                     || validate_literal<DayOfWeek>(match[6], day_of_week, day_names, 0);
        }
    }

    std::vector<std::string> CronTime::split(const std::string& s, char token)
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

    bool CronTime::is_number(const std::string& s)
    {
        // Find any character that isn't a number.
        return !s.empty()
               && std::find_if(s.begin(), s.end(),
                               [](char c)
                               { return !std::isdigit(c); }) == s.end();
    }

    bool CronTime::is_between(int32_t value, int32_t low_limit, int32_t high_limt)
    {
        return value >= low_limit && value <= high_limt;
    }
}