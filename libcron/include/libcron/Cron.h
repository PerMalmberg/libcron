#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <mutex>
#include <map>
#include <unordered_map>
#include <vector>
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

    template<typename ClockType, typename LockType, typename NameScheduleMapType>
    class Cron;

    template<typename ClockType, typename LockType, typename NameScheduleMapType>
    std::ostream& operator<<(std::ostream& stream, const Cron<ClockType, LockType, NameScheduleMapType>& c);

    template<typename ClockType = libcron::LocalClock, 
             typename LockType = libcron::NullLock, 
             typename NameScheduleMapType = std::map<std::string, std::string>>
    class Cron
    {
        public:            
            bool add_schedule(std::string name, const std::string& schedule, Task::TaskFunction work);
            bool add_schedule(const NameScheduleMapType& name_schedule_map, Task::TaskFunction work);
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
            class SortableQueue
            {
                public:
                    const std::vector<Task>& get_tasks() const
                    {
                        return c;
                    }

                    std::vector<Task>& get_tasks()
                    {
                        return c;
                    }

                    size_t size() const noexcept
                    {
                        return c.size();
                    }

                    bool empty() const noexcept
                    {
                        return c.empty();
                    }

                    void push(Task& t)
                    {
                        c.push_back(std::move(t));
                    }

                    void push(Task&& t)
                    {
                        c.push_back(std::move(t));
                    }

                    void push(std::vector<Task>&& tasks_to_insert)
                    {
                        c.reserve(c.size() + tasks_to_insert.size());
                        c.insert(c.end(), tasks_to_insert.begin(), tasks_to_insert.end());
                    }

                    const Task& top() const
                    {
                        return c[0];
                    }

                    Task& at(const size_t i)
                    {
                        return c[i];
                    }

                    void sort()
                    {
                        std::sort(c.begin(), c.end(), std::less<>());
                    }

                    void clear()
                    {
                        lock.lock();

                        std::vector<Task> empty;
                        c.swap(empty);

                        lock.unlock();
                    }

                    void remove(Task& to_remove)
                    {
                        auto it = std::find_if(c.begin(), c.end(), [&to_remove] (const Task& to_compare) { 
                                            return to_remove.get_name() == to_compare;
                                            });
                        
                        if (it != c.end())
                        {
                            c.erase(it);
                        }
                    }

                    void remove(std::string to_remove)
                    {
                        lock.lock();

                        auto it = std::find_if(c.begin(), c.end(), [&to_remove] (const Task& to_compare) { 
                                            return to_remove == to_compare;
                                            });

                        if (it != c.end())
                        {
                            c.erase(it);
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
                    std::vector<Task> c;
            };


            SortableQueue tasks{};
            ClockType clock{};
            bool first_tick = true;
            std::chrono::system_clock::time_point last_tick{};
    };
    
    template<typename ClockType, typename LockType, typename NameScheduleMapType>
    bool Cron<ClockType, LockType, NameScheduleMapType>::add_schedule(std::string name, const std::string& schedule, Task::TaskFunction work)
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
                tasks.sort();
            }
            tasks.release_queue();
        }

        return res;
    }

    template<typename ClockType, typename LockType, typename NameScheduleMapType>
    bool Cron<ClockType, LockType, NameScheduleMapType>::add_schedule(const NameScheduleMapType& name_schedule_map, Task::TaskFunction work)
    {
        bool res = false;
        std::vector<Task> tasks_to_add;
        tasks_to_add.reserve(name_schedule_map.size());

        for (const auto& [name, schedule] : name_schedule_map)
        {
            auto cron = CronData::create(schedule);
            res = cron.is_valid();
            if (res)
            {
                Task t{std::move(name), CronSchedule{cron}, work };
                if (t.calculate_next(clock.now()))
                {
                    tasks_to_add.push_back(std::move(t));
                }
            }
            else 
            {
                break;
            }
        }

        // Only add tasks and sort once if all elements in the map where valid
        if (res)
        {
            tasks.lock_queue();
            tasks.push(std::move(tasks_to_add));
            tasks.sort();
            tasks.release_queue();
        }
        return res;
    }

    template<typename ClockType, typename LockType, typename NameScheduleMapType>
    void Cron<ClockType, LockType, NameScheduleMapType>::clear_schedules()
    {
        tasks.clear();
    }
    
    template<typename ClockType, typename LockType, typename NameScheduleMapType>
    void Cron<ClockType, LockType, NameScheduleMapType>::remove_schedule(const std::string& name)
    {
        tasks.remove(name);
    }

    template<typename ClockType, typename LockType, typename NameScheduleMapType>
    std::chrono::system_clock::duration Cron<ClockType, LockType, NameScheduleMapType>::time_until_next() const
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

    template<typename ClockType, typename LockType, typename NameScheduleMapType>
    size_t Cron<ClockType, LockType, NameScheduleMapType>::tick(std::chrono::system_clock::time_point now)
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

        if (!tasks.empty())
        {
            for (size_t i = 0; i < tasks.size(); i++)
            {
                if (tasks.at(i).is_expired(now))
                {
                    auto& t = tasks.at(i);
                    t.execute(now);

                    using namespace std::chrono_literals;
                    if (!t.calculate_next(now + 1s))
                    {
                        tasks.remove(t);
                    }

                    res++;
                }
            }

            // Only Sort once and only if at least one task was executed
            if (res > 0)
            {
                tasks.sort();
            }
        }

        tasks.release_queue();
        return res;
    }

    template<typename ClockType, typename LockType, typename NameScheduleMapType>
    void Cron<ClockType, LockType, NameScheduleMapType>::get_time_until_expiry_for_tasks(std::vector<std::tuple<std::string,
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

    template<typename ClockType, typename LockType, typename NameScheduleMapType>
    std::ostream& operator<<(std::ostream& stream, const Cron<ClockType, LockType, NameScheduleMapType>& c)
    {
        std::for_each(c.tasks.get_tasks().cbegin(), c.tasks.get_tasks().cend(),
                      [&stream, &c](const Task& t)
                      {
                          stream << t.get_status(c.clock.now()) << '\n';
                      });

        return stream;
    }
}
