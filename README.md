# libcron
A C++ scheduling library using cron formatting.

# Using the Scheduler

Libcron offers an easy to use API to add callbacks with corresponding cron-formatted strings:

```
libcron::Cron cron;

cron.add_schedule("Hello from Cron", "* * * * * ?", [=](auto&) {
	std::cout << "Hello from libcron!" std::endl;
});
```

To trigger the execution of callbacks, one must call `libcron::Cron::tick` at least once a second to prevent missing schedules:

```
while(true)
{
	cron.tick();
	std::this_thread::sleep_for(500mS);
}
```

The callback must have the following signature:

```
std::function<void(const libcron::TaskInformation&)>
```

`libcron::Taskinformation` offers a convenient API to retrieve further information:

- `libcron::TaskInformation::get_delay` informs about the delay between planned and actual execution of the callback. Hence, it is possible to ensure that a task was executed within a specific tolerance:

```
libcron::Cron cron;

cron.add_schedule("Hello from Cron", "* * * * * ?", [=](auto& i) {
	using namespace std::chrono_literals;
	if (i.get_delay() >= 1s)
	{
		std::cout << "The Task was executed too late..." << std::endl;
	}
});
```



## Removing schedules from `libcron::Cron`

libcron::Cron offers two convenient functions to remove schedules:

- `clear_schedules()` will remove all schedules
- `remove_schedule(std::string)` will remove a specific schedule

For example, `cron.remove_schedule("Hello from Cron")` will remove the previously added task.



## Removing/Adding tasks at runtime in a multithreaded environment

When Calling `libcron::Cron::tick` from another thread than `add_schedule`, `clear_schedule` and `remove_schedule`, one must take care to protect the internal resources of `libcron::Cron` so that tasks are not removed or added while `libcron::Cron` is iterating over the schedules. `libcron::Cron` can take care of that, you simply have to define your own aliases:

```
/* The default class uses NullLock, which does not lock the resources at runtime */
template<typename ClockType = libcron::LocalClock, typename LockType = libcron::NullLock>
class Cron
{
	...
}

/* Define an alias for a thread-safe Cron scheduler which automatically locks ressources when needed */ 
using CronMt = libcron::Cron<libcron::LocalClock, libcron::Locker>

CronMt cron;
cron.add_schedule("Hello from Cron", "* * * * * ?", [=]() {
	std::cout << "Hello from CronMt!" std::endl;
});

....
```

However, this comes with costs: Whenever you call `tick`, a `std::mutex` will be locked and unlocked.  So only use the `libcron::Locker` to protect resources when you really need too.

## Local time vs UTC

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
the '?'-character to ensure that it is not possible to specify a statement which results in an impossible mix of these fields. 

## Examples

|Expression | Meaning
| --- | --- |
| * * * * * ? | Every second
| 0 0 12 * * MON-FRI | Every Weekday at noon
| 0 0 12 1/2 * ?	| Every 2 days, starting on the 1st at noon
| 0 0 */12 ? * * | Every twelve hours

# Randomization

The standard cron format does not allow for randomization, but with the use of `CronRandomization` you can generate random
schedules using the following format: `R(range_start-range_end)`, where `range_start` and `range_end` follow the same rules
as for a regular cron range (step-syntax is not supported). All the rules for a regular cron expression still applies
when using randomization, i.e. mutual exclusiveness and no extra spaces.

## Examples
|Expression | Meaning
| --- | --- |
| 0 0 R(13-20) * * ? | On the hour, on a random hour 13-20, inclusive.
| 0 0 0 ? * R(0-6) | A random weekday, every week, at midnight.
| 0 R(45-15) */12 ? * * | A random minute between 45-15, inclusive, every 12 hours.
|0 0 0 ? R(DEC-MAR) R(SAT-SUN)| On the hour, on a random month december to march, on a random weekday saturday to sunday. 


# Used Third party libraries

Howard Hinnant's [date libraries](https://github.com/HowardHinnant/date/)
