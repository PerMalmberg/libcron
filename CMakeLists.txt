cmake_minimum_required(VERSION 3.6)

add_subdirectory(libcron)
add_subdirectory(test)

add_dependencies(cron_test libcron)