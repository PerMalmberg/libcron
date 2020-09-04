#pragma once

#include <functional>
#include <chrono>

namespace libcron
{
    class Task;
    
    template<typename WorkType>
    class TaskContextProxyImplementation;

    // The Context a callback may receive
    class TaskContext
    {
        public:
            // The actual valid callback signatures
            using work_type_without_context = std::function<void()>;
            using work_type_with_context = std::function<void(TaskContext&)>;

            TaskContext(work_type_without_context work) : expects_context(false), work_without_context(std::move(work)) { }
            TaskContext(work_type_with_context work) : expects_context(true), work_with_context(std::move(work)) { }
            
            void operator() (Task* t)
            {
                if (expects_context)
                {
                    task = t;
                    work_with_context(*this);
                }
                else
                {
                    work_without_context();
                }
            }

            std::chrono::system_clock::duration get_delay();

        private:
            Task* task;
            bool expects_context;
            work_type_without_context work_without_context;
            work_type_with_context work_with_context;
    };
}
