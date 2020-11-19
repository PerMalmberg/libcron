cmake_minimum_required(VERSION 3.6)
project(libcron)

set(CMAKE_CXX_STANDARD 17)

# Deactivate Iterator-Debugging on Windows
option(LIBCRON_DEACTIVATE_ITERATOR_DEBUGGING "Build with iterator-debugging (MSVC only)." OFF)

if( MSVC )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	
	if (LIBCRON_DEACTIVATE_ITERATOR_DEBUGGING)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_HAS_ITERATOR_DEBUGGING=0")
	endif()
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

add_library(${PROJECT_NAME}
		include/libcron/Cron.h
		include/libcron/CronClock.h
		include/libcron/CronData.h
		include/libcron/CronRandomization.h
		include/libcron/CronSchedule.h
		include/libcron/DateTime.h
		include/libcron/Task.h
		include/libcron/TimeTypes.h
		src/CronClock.cpp
		src/CronData.cpp
		src/CronRandomization.cpp
		src/CronSchedule.cpp
		src/Task.cpp)

target_include_directories(${PROJECT_NAME}
		PRIVATE ${CMAKE_CURRENT_LIST_DIR}/externals/date/include
		PUBLIC include)

if(NOT MSVC)
	# Assume a modern compiler (gcc 9.3)
	target_compile_definitions (${PROJECT_NAME} PRIVATE -DHAS_UNCAUGHT_EXCEPTIONS)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/out/${CMAKE_BUILD_TYPE}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/out/${CMAKE_BUILD_TYPE}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/out/${CMAKE_BUILD_TYPE}")
