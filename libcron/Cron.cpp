#include <functional>
#include "Cron.h"

using namespace std::chrono;

namespace libcron
{
    bool libcron::Cron::add_schedule( std::string name, const std::string& schedule, std::function<void()> work)
    {
        auto cron = CronData::create(schedule);
        bool res = cron.is_valid();
        if (res)
        {

            Task t{std::move(name), CronSchedule{cron}, std::move(work)};
            if (t.calculate_next())
            {
                tasks.push(t);
            }
        }

        return res;
    }

    std::chrono::system_clock::duration Cron::time_until_next(std::chrono::system_clock::time_point now) const
    {
        system_clock::duration d{};
        if (tasks.empty())
        {
            d = std::numeric_limits<minutes>::max();
        }
        else
        {
            d = tasks.top().time_until_expiry(now);
        }

        return d;
    }

    size_t Cron::execute_expired_tasks(system_clock::time_point now)
    {
        std::vector<Task> executed{};

        while(!tasks.empty()
              && tasks.top().is_expired(now))
        {
            executed.push_back(tasks.top());
            tasks.pop();
            auto& t = executed[executed.size()-1];
            t.execute();
        }

        auto res = executed.size();

        // Place executed tasks back onto the priority queue.
        std::for_each(executed.begin(), executed.end(), [this, &now](Task& task)
        {
            // Must calculate new schedules using second after 'now', otherwise
            // we'll run the same task over and over if it takes less than 1s to execute.
            if(task.calculate_next(now + 1s))
            {
                tasks.push(task);
            }
        });

        print_queue(tasks);

        return res;
    }

    void Cron::print_queue(std::priority_queue<Task, std::vector<Task>, std::greater<>> queue)
    {
        std::vector<Task> v{};

        while( !queue.empty())
        {
            auto t = queue.top();
            queue.pop();
            v.push_back(t);
        }

        std::for_each(v.begin(), v.end(), [&queue](auto& task){
            queue.push(task);
        });
    }

}