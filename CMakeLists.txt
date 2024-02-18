cmake_minimum_required(VERSION 3.6)

project(top)

option(LIBCRON_BUILD_TZ_CLOCK "Build timezone clock" OFF)

if(LIBCRON_BUILD_TZ_CLOCK)
    set(BUILD_TZ_LIB ON CACHE BOOL "Build the tz library" FORCE)
    add_subdirectory(libcron/externals/date)
endif()

add_subdirectory(libcron)
add_subdirectory(test)

add_dependencies(cron_test libcron)

install(TARGETS libcron DESTINATION lib)
install(DIRECTORY libcron/include/libcron DESTINATION include)
install(DIRECTORY libcron/externals/date/include/date DESTINATION include)
