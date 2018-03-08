cmake_minimum_required(VERSION 3.9)
project(libcron)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -fsanitize=address")

add_library(${PROJECT_NAME}
        Cron.h
        Cron.cpp Task.h CronTime.h TimeTypes.h CronTime.cpp)
