#pragma once

#include <chrono>

namespace libcron
{
    class ICronClock
    {
        public:
            virtual std::chrono::system_clock::time_point now() = 0;
    };

    class UTCClock
            : public ICronClock
    {
        public:
            std::chrono::system_clock::time_point now() override
            {
                return std::chrono::system_clock::now();
            }
    };
}