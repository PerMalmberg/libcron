cmake_minimum_required(VERSION 3.6)

project(top)
add_subdirectory(libcron)
add_subdirectory(test)

add_dependencies(cron_test libcron)

install(TARGETS libcron DESTINATION lib)
install(DIRECTORY libcron/include/libcron DESTINATION include)
install(DIRECTORY libcron/externals/date/include/date DESTINATION include)

