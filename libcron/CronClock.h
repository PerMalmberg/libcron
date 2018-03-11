#pragma once

#include <chrono>

using namespace std::chrono;
using namespace date;

namespace libcron
{
    class ICronClock
    {
        public:
            virtual std::chrono::system_clock::time_point now() = 0;
            virtual std::chrono::seconds utc_offset(std::chrono::system_clock::time_point now) = 0;
    };

    class UTCClock
            : public ICronClock
    {
        public:
            std::chrono::system_clock::time_point now() override
            {
                return std::chrono::system_clock::now();
            }

            std::chrono::seconds utc_offset(std::chrono::system_clock::time_point) override
            {
                return 0s;
            }
    };

    class LocalClock
            : public ICronClock
    {
        public:
            std::chrono::system_clock::time_point now() override
            {
                auto now = system_clock::now();
                return now + utc_offset(now);
            }

            std::chrono::seconds utc_offset(std::chrono::system_clock::time_point now) override
            {
                auto t = system_clock::to_time_t(now);
                tm tm{};
#ifdef __WIN32
                localtime_s(&tm, &t);
#else
                localtime_r(&t, &tm);
#endif
                return seconds{tm.tm_gmtoff};
            }
    };
}