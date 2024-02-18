#pragma once

#include <chrono>
#ifdef BUILD_TZ_CLOCK
#include <date/tz.h>
#include <mutex>
#endif

namespace libcron
{
    class ICronClock
    {
        public:
            virtual std::chrono::system_clock::time_point now() const = 0;
            virtual std::chrono::seconds utc_offset(std::chrono::system_clock::time_point now) const = 0;
    };

    class UTCClock
            : public ICronClock
    {
        public:
            std::chrono::system_clock::time_point now() const override
            {
                return std::chrono::system_clock::now();
            }

            std::chrono::seconds utc_offset(std::chrono::system_clock::time_point) const override
            {
				using namespace std::chrono;
                return 0s;
            }
    };

    class LocalClock
            : public ICronClock
    {
        public:
            std::chrono::system_clock::time_point now() const override
            {
                auto now = std::chrono::system_clock::now();
                return now + utc_offset(now);
            }

			std::chrono::seconds utc_offset(std::chrono::system_clock::time_point now) const override;
    };

#ifdef BUILD_TZ_CLOCK
    class TzClock : public ICronClock
    {
        public:
            std::chrono::system_clock::time_point now() const override
            {
                auto now = std::chrono::system_clock::now();
                return now + utc_offset(now);
            }

            bool set_time_zone(std::string_view tz_name);
            std::chrono::seconds utc_offset(std::chrono::system_clock::time_point now) const override;
        private:
            mutable std::mutex time_zone_mtx{};
            const date::time_zone* time_zone{nullptr};
    };
#endif
}
