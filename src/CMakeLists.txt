cmake_minimum_required (VERSION 2.6)
project (rainconf)

INCLUDE(CheckFunctionExists)
INCLUDE(CheckIncludeFiles)

# flock
check_include_files(sys/file.h HAVE_SYS_FILE_HEADER)
CHECK_FUNCTION_EXISTS(flock HAVE_FLOCK)

# write env.h
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/env.h.in ${CMAKE_CURRENT_SOURCE_DIR}/env.h)

add_executable(rainconf rainconf.cpp RainAdapter.cpp serialport.cpp)

find_package(P8-platform REQUIRED)

find_package(Threads REQUIRED)

target_link_libraries(rainconf ${p8-platform_LIBRARIES})
target_link_libraries(rainconf ${CMAKE_THREAD_LIBS_INIT})

