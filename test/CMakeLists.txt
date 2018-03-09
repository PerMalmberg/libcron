cmake_minimum_required(VERSION 3.6)
project(cron_test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -fsanitize=address -lasan")

include_directories(
        externals/Catch2/single_include/
        ..
        ../libcron/externals/date
)

add_executable(
        ${PROJECT_NAME}
        test.cpp
)

target_link_libraries(${PROJECT_NAME} libcron)