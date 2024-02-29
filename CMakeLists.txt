cmake_minimum_required(VERSION 3.6)

project(top)

option(LIBCRON_BUILD_TZ_CLOCK "Build timezone clock" OFF)
option(LIBCRON_MANUAL_TZ_DB "User will set TZ DB manually by invoking date::set_install in their code" OFF)

if(LIBCRON_BUILD_TZ_CLOCK)
    add_library(date-tz)
    target_sources(date-tz
      PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/libcron/externals/date/include/date/tz.h
      PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/libcron/externals/date/include/date/tz_private.h
        ${CMAKE_CURRENT_LIST_DIR}/libcron/externals/date/src/tz.cpp
    )
    target_include_directories(date-tz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libcron/externals/date/include)

    if (LIBCRON_MANUAL_TZ_DB)
        target_compile_definitions(date-tz PRIVATE AUTO_DOWNLOAD=0 HAS_REMOTE_API=0)
    else()
        find_package(CURL REQUIRED)
        target_link_libraries(date-tz PRIVATE CURL::libcurl)
        target_compile_definitions(date-tz PRIVATE AUTO_DOWNLOAD=1 HAS_REMOTE_API=1)
    endif()

    if (WIN32 AND BUILD_SHARED_LIBS)
        target_compile_definitions(date-tz PUBLIC DATE_BUILD_DLL=1)
    endif()

    if(NOT MSVC)
        find_package(Threads)
        target_link_libraries(date-tz PUBLIC Threads::Threads)
    endif()
endif()

add_subdirectory(libcron)
add_subdirectory(test)

add_dependencies(cron_test libcron)

install(TARGETS libcron DESTINATION lib)
install(DIRECTORY libcron/include/libcron DESTINATION include)
install(DIRECTORY libcron/externals/date/include/date DESTINATION include)
