cmake_minimum_required(VERSION 3.6)
project(libcron)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

include_directories(externals/date)

add_library(${PROJECT_NAME}
        Cron.h
        Cron.cpp
        Task.h
        CronData.h
        TimeTypes.h
        CronData.cpp
        CronSchedule.cpp
        CronSchedule.h
        externals/date/date.h
        DateTime.h
        Task.cpp)
