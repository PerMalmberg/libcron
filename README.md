# libcron
A C++ scheduling library using cron formatting.

# Local time vs UTC

This library uses `std::chrono::system_clock::timepoint` as its time unit. While that is UTC by default, the Cron-class
uses a `LocalClock` by default which offsets `system_clock::now()` by the current UTC-offset. If you wish to work in
UTC, then construct the Cron instance, passing it a `libcron::UTCClock`.  

# Supported formatting

This implementation supports cron format, as specified below.  

Each schedule expression conststs of 6 parts, all mandatory. However, if 'day of month' specifies specific days, then 'day of week' is ignored.

```text
┌──────────────seconds (0 - 59)
│ ┌───────────── minute (0 - 59)
│ │ ┌───────────── hour (0 - 23)
│ │ │ ┌───────────── day of month (1 - 31)
│ │ │ │ ┌───────────── month (1 - 12)
│ │ │ │ │ ┌───────────── day of week (0 - 6) (Sunday to Saturday)
│ │ │ │ │ │
│ │ │ │ │ │
│ │ │ │ │ │
* * * * * *
```
* Allowed formats:
  * Special characters: '*', meaning the entire range.
  * '?' used to ignore day of month/day of week as noted below.

  * Ranges: 1,2,4-6
    * Result: 1,2,4,5,6
  * Steps: n/m, where n is the start and m is the step.
    * `1/2` yields 1,3,5,7...<max>
    * `5/3` yields 5,8,11,14...<max>
    * `*/2` yields Result: 1,3,5,7...<max>
  * Reversed ranges:
    * `0 0 23-2 * * *`, meaning top of each minute and hour, of hours, 23, 0, 1 and 2, every day.
      * Compare to `0 0 2-23 * * *` which means top of each minute and hour, of hours, 2,3...21,22,23 every day.



For `month`, these (case insensitive) strings can be used instead of numbers: `JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC`.
Example: `JAN,MAR,SEP-NOV`

For `day of week`, these (case insensitive) strings can be used instead of numbers: `SUN, MON, TUE, WED, THU, FRI, SAT`. 
Example: `MON-THU,SAT`

Each part is separated by one or more whitespaces. It is thus important to keep whitespaces out of the respective parts.

* Valid:
  * 0,3,40-50 * * * * ?

* Invalid:
  * 0, 3, 40-50 * * * * ?
  
`Day of month` and `day of week` are mutually exclusive so one of them must at always be ignored using
the '?'-character unless one field already is something other than '*'. 

# Examples

|Expression | Meaning
| --- | --- |
| * * * * * ? | Every second
|0 0 12 * * MON-FRI | Every Weekday at noon
|0 0 12 1/2 * ?	| Every 2 days, starting on the 1st at noon
| 0 0 */12 ? * * | Every twelve hours

# Third party libraries

Howard Hinnant's [date libraries](https://github.com/HowardHinnant/date/)

