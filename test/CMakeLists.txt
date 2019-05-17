cmake_minimum_required(VERSION 3.6)
project(cron_test)

set(CMAKE_CXX_STANDARD 17)

if( MSVC )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

include_directories(
        ${CMAKE_CURRENT_LIST_DIR}/externals/Catch2/single_include/
        ${CMAKE_CURRENT_LIST_DIR}/../libcron/externals/date/include
        ${CMAKE_CURRENT_LIST_DIR}/..
)

add_executable(
        ${PROJECT_NAME}
        CronDataTest.cpp
        CronRandomizationTest.cpp
		CronScheduleTest.cpp
		CronTest.cpp)

target_link_libraries(${PROJECT_NAME} libcron)

set_target_properties(${PROJECT_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/out"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/out"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/out")