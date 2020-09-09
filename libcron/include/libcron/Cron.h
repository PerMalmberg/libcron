#pragma once

#include <string>
#include <chrono>
#include <queue>
#include <memory>
#include <mutex>
#include "Task.h"
#include "CronClock.h"

namespace libcron
{
    class NullLock 
    {
        public:
            void lock() {}
            void unlock() {}
    };

    class Locker
    {
        public:
            void lock() { m.lock(); }
            void unlock() { m.unlock(); }
        private:
            std::recursive_mutex m{};
    };

    template<typename ClockType, typename LockType>
    class Cron;

    template<typename ClockType, typename LockType>
    std::ostream& operator<<(std::ostream& stream, const Cron<ClockType, LockType>& c);

    template<typename ClockType = libcron::LocalClock, typename LockType = libcron::NullLock>
    class Cron
    {
        public:
            bool add_schedule(std::string name, const std::string& schedule, Task::TaskFunction work);
            void clear_schedules();
            void remove_schedule(const std::string& name);

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

            void get_time_until_expiry_for_tasks(
                    std::vector<std::tuple<std::string, std::chrono::system_clock::duration>>& status) const;

            friend std::ostream& operator<<<>(std::ostream& stream, const Cron<ClockType, LockType>& c);

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

                    void clear()
                    {
                        lock.lock();

                        while (!empty())
                            pop();

                        lock.unlock();
                    }

                    template<typename T>
                    void remove(T to_remove)
                    {
                        lock.lock();

                        /* Copy current elements */
                        std::vector<Task> temp{};
                        std::swap(temp, c);

                        /* Refill with elements ensuring correct order by calling push */
                        for (const auto& task : temp)
                        {
                            if (task != to_remove)
                                push(task);
                        }

                        lock.unlock();
                    }

                    void lock_queue()
                    {
                        /* Do not allow to manipulate the Queue */
                        lock.lock();
                    }

                    void release_queue()
                    {
                        /* Allow Access to the Queue Manipulating-Functions */
                        lock.unlock();
                    }
                    
                private:
                    LockType lock;
            };


            Queue tasks{};
            ClockType clock{};
            bool first_tick = true;
            std::chrono::system_clock::time_point last_tick{};
    };
    
    template<typename ClockType, typename LockType>
    bool Cron<ClockType, LockType>::add_schedule(std::string name, const std::string& schedule, Task::TaskFunction work)
    {
        auto cron = CronData::create(schedule);
        bool res = cron.is_valid();
        if (res)
        {
            tasks.lock_queue();
            Task t{std::move(name), CronSchedule{cron}, work };
            if (t.calculate_next(clock.now()))
            {
                tasks.push(t);
            }
            tasks.release_queue();
        }

        return res;
    }

    template<typename ClockType, typename LockType>
    void Cron<ClockType, LockType>::clear_schedules()
    {
        tasks.clear();
    }
    
    template<typename ClockType, typename LockType>
    void Cron<ClockType, LockType>::remove_schedule(const std::string& name)
    {
        tasks.remove(name);
    }

    template<typename ClockType, typename LockType>
    std::chrono::system_clock::duration Cron<ClockType, LockType>::time_until_next() const
    {
        std::chrono::system_clock::duration d{};
        if (tasks.empty())
        {
            d = std::numeric_limits<std::chrono::minutes>::max();
        }
        else
        {
            d = tasks.top().time_until_expiry(clock.now());
        }

        return d;
    }

    template<typename ClockType, typename LockType>
    size_t Cron<ClockType, LockType>::tick(std::chrono::system_clock::time_point now)
    {
        tasks.lock_queue();
        size_t res = 0;

        if(!first_tick)
        {
            // Only allow time to flow if at least one second has passed since the last tick,
            // either forward or backward.
            auto diff = now - last_tick;

            constexpr auto one_second = std::chrono::seconds{1};

            if(diff < one_second && diff > -one_second)
            {
                now = last_tick;
            }
        }




        if (first_tick)
        {
            first_tick = false;
        }
        else
        {
            // https://linux.die.net/man/8/cron

            constexpr auto three_hours = std::chrono::hours{3};
            auto diff = now - last_tick;
            auto absolute_diff = diff > diff.zero() ? diff : -diff;

            if(absolute_diff >= three_hours)
            {
                // Time changes of more than 3 hours are considered to be corrections to the
                // clock or timezone, and the new time is used immediately.
                for (auto& t : tasks.get_tasks())
                {
                    t.calculate_next(now);
                }
            }
            else
            {
                // Change of less than three hours

                // If time has moved backwards: Since tasks are not rescheduled, they won't run before
                // we're back at least the original point in time which prevents running tasks twice.

                // If time has moved forward, tasks that would have run since last tick will be run.
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
            using namespace std::chrono_literals;
            if (task.calculate_next(now + 1s))
            {
                tasks.push(task);
            }
        });

        tasks.release_queue();
        return res;
    }

    template<typename ClockType, typename LockType>
    void Cron<ClockType, LockType>::get_time_until_expiry_for_tasks(std::vector<std::tuple<std::string,
                                                          std::chrono::system_clock::duration>>& status) const
    {
        auto now = clock.now();
        status.clear();

        std::for_each(tasks.get_tasks().cbegin(), tasks.get_tasks().cend(),
                      [&status, &now](const Task& t)
                      {
                          status.emplace_back(t.get_name(), t.time_until_expiry(now));
                      });
    }

    template<typename ClockType, typename LockType>
    std::ostream& operator<<(std::ostream& stream, const Cron<ClockType, LockType>& c)
    {
        std::for_each(c.tasks.get_tasks().cbegin(), c.tasks.get_tasks().cend(),
                      [&stream, &c](const Task& t)
                      {
                          stream << t.get_status(c.clock.now()) << '\n';
                      });

        return stream;
    }
}
