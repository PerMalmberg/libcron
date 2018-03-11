cmake_minimum_required(VERSION 3.6)
project(cron_test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -Wextra")

include_directories(
        ${CMAKE_CURRENT_LIST_DIR}/externals/Catch2/single_include/
        ${CMAKE_CURRENT_LIST_DIR}/../libcron/externals/date/include
        ${CMAKE_CURRENT_LIST_DIR}/..
)

add_executable(
        ${PROJECT_NAME}
        CronDataTest.cpp
        CronScheduleTest.cpp CronTest.cpp)

target_link_libraries(${PROJECT_NAME} libcron)