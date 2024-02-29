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
    template<typename LockType = std::mutex>
    class TzClock : public ICronClock
    {
        public:
            std::chrono::system_clock::time_point now() const override
            {
                auto now = std::chrono::system_clock::now();
                return now + utc_offset(now);
            }

            std::chrono::seconds utc_offset(std::chrono::system_clock::time_point now) const override
            {
                // If we don't have a timezone we use utc
                using namespace std::chrono;
                std::lock_guard<LockType> lock(time_zone_mtx);
                if (time_zone)
                    return time_zone->get_info(now).offset;
                else
                    return 0s;
            }

            bool set_time_zone(std::string_view tz_name)
            {
                const date::time_zone *new_zone{nullptr};
                try
                {
                    new_zone = date::locate_zone(tz_name);
                }
                catch (std::runtime_error &err)
                {
                    return false;
                }
                std::lock_guard<LockType> lock(time_zone_mtx);
                time_zone = new_zone;
                return true;
            }
        private:
            mutable LockType time_zone_mtx{};
            const date::time_zone* time_zone{nullptr};
    };
#endif
}
