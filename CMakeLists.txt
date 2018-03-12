cmake_minimum_required(VERSION 3.6)

set(OUTPUT_LOCATION ${CMAKE_CURRENT_LIST_DIR}/out/)

add_subdirectory(libcron)
add_subdirectory(test)

add_dependencies(cron_test libcron)

