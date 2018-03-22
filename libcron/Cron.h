#pragma once

#include <string>
#include <chrono>
#include <queue>
#include <memory>
#include "Task.h"
#include "CronClock.h"

namespace libcron
{
    template<typename ClockType>
    class Cron;

    template<typename ClockType>
    std::ostream& operator<<(std::ostream& stream, const Cron<ClockType>& c);

    template<typename ClockType = libcron::LocalClock>
    class Cron
    {
        public:

            bool add_schedule(std::string name, const std::string& schedule, std::function<void()> work);

            size_t count() const
            {
                return tasks.size();
            }

            // Tick is expected to be called at least once a second to prevent missing schedules.
            size_t
            tick()
            {
                return tick(clock.now());
            }

            size_t
            tick(std::chrono::system_clock::time_point now);

            std::chrono::system_clock::duration
            time_until_next() const;

            ClockType& get_clock()
            {
                return clock;
            }

			void get_time_until_expiry_for_tasks(std::vector<std::tuple<std::string, std::chrono::system_clock::duration>>& status) const;

            friend std::ostream& operator<<<>(std::ostream& stream, const Cron<ClockType>& c);

        private:
            class Queue
                    // Priority queue placing smallest (i.e. nearest in time) items on top.
                    : public std::priority_queue<Task, std::vector<Task>, std::greater<>>
            {
                public:
                    // Inherit to allow access to the container.
                    const std::vector<Task>& get_tasks() const
                    {
                        return c;
                    }

                    std::vector<Task>& get_tasks()
                    {
                        return c;
                    }
            };

            Queue tasks{};
            ClockType clock{};
            bool first_tick = true;
            std::chrono::system_clock::time_point last_tick{};
    };

    template<typename ClockType>
    bool Cron<ClockType>::add_schedule(std::string name, const std::string& schedule, std::function<void()> work)
    {
        auto cron = CronData::create(schedule);
        bool res = cron.is_valid();
        if (res)
        {
            Task t{std::move(name), CronSchedule{cron}, std::move(work)};
            if (t.calculate_next(clock.now()))
            {
                tasks.push(t);
            }
        }

        return res;
    }

    template<typename ClockType>
    std::chrono::system_clock::duration Cron<ClockType>::time_until_next() const
    {
        system_clock::duration d{};
        if (tasks.empty())
        {
            d = std::numeric_limits<minutes>::max();
        }
        else
        {
            d = tasks.top().time_until_expiry(clock.now());
        }

        return d;
    }

    template<typename ClockType>
    size_t Cron<ClockType>::tick(std::chrono::system_clock::time_point now)
    {
        size_t res = 0;

        if (first_tick)
        {
            first_tick = false;
        }
        else if (now > last_tick && now - last_tick <= hours{3})
        {
            // Reschedule all tasks.
            for (auto& t : tasks.get_tasks())
            {
                t.calculate_next(now);
            }
        }
        else if (now < last_tick && now - last_tick <= -hours{3})
        {
            // Reschedule all tasks.
            for (auto& t : tasks.get_tasks())
            {
                t.calculate_next(now);
            }
        }

        last_tick = now;

        std::vector<Task> executed{};

        while (!tasks.empty()
               && tasks.top().is_expired(now))
        {
            executed.push_back(tasks.top());
            tasks.pop();
            auto& t = executed[executed.size() - 1];
            t.execute(now);
        }

        res = executed.size();

        // Place executed tasks back onto the priority queue.
        std::for_each(executed.begin(), executed.end(), [this, &now](Task& task)
        {
            // Must calculate new schedules using second after 'now', otherwise
            // we'll run the same task over and over if it takes less than 1s to execute.
            if (task.calculate_next(now + 1s))
            {
                tasks.push(task);
            }
        });

        return res;
    }

	template<typename ClockType>
	void Cron<ClockType>::get_time_until_expiry_for_tasks(std::vector<std::tuple<std::string, std::chrono::system_clock::duration>>& status) const
	{
		auto now = clock.now();
		status.clear();

		std::for_each(tasks.get_tasks().cbegin(), tasks.get_tasks().cend(),
			[&status, &now](const Task& t)
		{
			status.emplace_back(t.get_name(), t.time_until_expiry(now));
		});
	}

    template<typename ClockType>
    std::ostream& operator<<(std::ostream& stream, const Cron<ClockType>& c)
    {
        std::for_each(c.tasks.get_tasks().cbegin(), c.tasks.get_tasks().cend(),
                      [&stream, &c](const Task& t)
                      {
                          stream << t.get_status(c.clock.now()) << '\n';
                      });

        return stream;
    }
}