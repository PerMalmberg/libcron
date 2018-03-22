cmake_minimum_required(VERSION 3.6)
project(libcron)

set(CMAKE_CXX_STANDARD 14)

if( MSVC )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/externals/date/include)

add_library(${PROJECT_NAME}
        Cron.h
        Task.h
        CronData.h
        TimeTypes.h
        CronData.cpp
        CronSchedule.cpp
        CronSchedule.h
        DateTime.h
        Task.cpp
        CronClock.h
		CronClock.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_LOCATION}"
        LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_LOCATION}"
        RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_LOCATION}")