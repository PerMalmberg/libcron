#include "libcron/CronClock.h"

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#endif

using namespace std::chrono;

namespace libcron
{

	std::chrono::seconds LocalClock::utc_offset(std::chrono::system_clock::time_point now) const
	{
#ifdef WIN32
		(void)now;

		TIME_ZONE_INFORMATION tz_info{};
		seconds offset{ 0 };

		auto res = GetTimeZoneInformation(&tz_info);
		if (res != TIME_ZONE_ID_INVALID)
		{
			// https://msdn.microsoft.com/en-us/library/windows/desktop/ms725481(v=vs.85).aspx
			// UTC = local time + bias => local_time = utc - bias, so UTC offset is -bias
			offset = minutes{ -tz_info.Bias };
		}
#else
		auto t = system_clock::to_time_t(now);
		tm tm{};
		localtime_r(&t, &tm);
		seconds offset{ tm.tm_gmtoff };
#endif
		return offset;
	}

#ifdef BUILD_TZ_CLOCK
	bool TzClock::set_time_zone(std::string_view tz_name)
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

		std::lock_guard<std::mutex> lock(time_zone_mtx);
		time_zone = new_zone;
		return true;
	}

	std::chrono::seconds TzClock::utc_offset(std::chrono::system_clock::time_point now) const
	{
		using namespace std::chrono;
		// If we don't have a timezone we use utc
		std::lock_guard<std::mutex> lock(time_zone_mtx);
		if (time_zone)
			return time_zone->get_info(now).offset;
		else
			return 0s;
	}
#endif
}