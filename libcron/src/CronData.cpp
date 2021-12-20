#include <date/date.h>
#include "libcron/CronData.h"

using namespace date;

namespace libcron
{
    const constexpr Months CronData::months_with_31[NUMBER_OF_LONG_MONTHS] = { Months::January,
                                                                               Months::March,
                                                                               Months::May,
                                                                               Months::July,
                                                                               Months::August,
                                                                               Months::October,
                                                                               Months::December };

    const std::vector<std::string> CronData::month_names{ "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
    const std::vector<std::string> CronData::day_names{ "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
    std::unordered_map<std::string, CronData> CronData::cache{};

    CronData CronData::create(const std::string& cron_expression)
    {
        CronData c;
        auto found = cache.find(cron_expression);

        if (found == cache.end())
        {
            c.parse(cron_expression);
            cache[cron_expression] = c;
        }
        else
        {
            c = found->second;
        }
        
        return c;
    }

    void CronData::parse(const std::string& cron_expression)
    {
        // First, check for "convenience scheduling" using @yearly, @annually,
        // @monthly, @weekly, @daily or @hourly.
        std::string tmp = std::regex_replace(cron_expression, std::regex("@yearly"), "0 0 1 1 *");
        tmp = std::regex_replace(tmp, std::regex("@annually"), "0 0 1 1 *");
        tmp = std::regex_replace(tmp, std::regex("@monthly"), "0 0 1 * *");
        tmp = std::regex_replace(tmp, std::regex("@weekly"), "0 0 * * 0");
        tmp = std::regex_replace(tmp, std::regex("@daily"), "0 0 * * *");
        const std::string expression = std::regex_replace(tmp, std::regex("@hourly"), "0 * * * *");

        // Second, split on white-space. We expect six parts.
        std::regex split{ R"#(^\s*(.*?)\s+(.*?)\s+(.*?)\s+(.*?)\s+(.*?)\s+(.*?)\s*$)#",
                          std::regex_constants::ECMAScript };

        std::smatch match;

        if (std::regex_match(expression.begin(), expression.end(), match, split))
        {
            valid = validate_numeric<Seconds>(match[1], seconds);
            valid &= validate_numeric<Minutes>(match[2], minutes);
            valid &= validate_numeric<Hours>(match[3], hours);
            valid &= validate_numeric<DayOfMonth>(match[4], day_of_month);
            valid &= validate_literal<Months>(match[5], months, month_names);
            valid &= validate_literal<DayOfWeek>(match[6], day_of_week, day_names);
            valid &= check_dom_vs_dow(match[4], match[6]);
            valid &= validate_date_vs_months();
        }
    }

    std::vector<std::string> CronData::split(const std::string& s, char token)
    {
        std::vector<std::string> res;

        std::string r = "[";
        r += token;
        r += "]";
        std::regex splitter{ r, std::regex_constants::ECMAScript };

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

        if (res)
        {
            // Make sure that if the days contains only 31, at least one month allows that date.
            if (day_of_month.size() == 1 && day_of_month.find(DayOfMonth::Last) != day_of_month.end())
            {
                res = false;

                for (size_t i = 0; !res && i < NUMBER_OF_LONG_MONTHS; ++i)
                {
                    res = months.find(months_with_31[i]) != months.end();
                }
            }
        }

        return res;
    }

    bool CronData::check_dom_vs_dow(const std::string& dom, const std::string& dow) const
    {
        // Day of month and day of week are mutually exclusive so one of them must at always be ignored using
        // the '?'-character unless one field already is something other than '*'.
        //
        // Since we treat an ignored field as allowing the full range, we're OK with both being flagged
        // as ignored. To make it explicit to the user of the library, we do however require the use of
        // '?' as the ignore flag, although it is functionally equivalent to '*'.

        auto check = [](const std::string& l, std::string r)
                     {
                         return l == "*" && (r != "*" || r == "?");
                     };

        return (dom == "?" || dow == "?")
               || check(dom, dow)
               || check(dow, dom);
    }
}
