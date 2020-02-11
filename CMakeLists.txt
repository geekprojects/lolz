cmake_minimum_required(VERSION 3.10)
project(lolz VERSION 1.0)

include(CheckIncludeFile)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)

pkg_check_modules(libgeek REQUIRED libgeek)
pkg_check_modules(yamlcpp REQUIRED yaml-cpp)

find_library(fsw_LIBRARY NAMES fswatch )
find_path(fsw_INCLUDE_BASE NAMES libfswatch/c++/monitor.hpp)
find_path(HAS_FSW_MONITOR_FACTORY NAMES libfswatch/c++/monitor_factory.hpp)

if (HAS_FSW_MONITOR_FACTORY)
    add_definitions(-DHAS_FSW_MONITOR_FACTORY)
endif()

include_directories(
    ${fsw_INCLUDE_BASE}
)

add_definitions(${libgeek_CFLAGS} ${yamlcpp_CFLAGS})

add_executable(lolzcat
    src/lolzcat/main.cpp
    src/lolzcat/lolz.cpp
    src/lolzcat/logdirectory.cpp
    src/lolzcat/logfile.cpp
)
target_link_libraries(lolzcat ${libgeek_LDFLAGS} ${fsw_LIBRARY} ${yamlcpp_LDFLAGS})

add_executable(lolzfindr
    src/lolzfindr/main.cpp
)
target_link_libraries(lolzfindr ${libgeek_LDFLAGS} ${yamlcpp_LDFLAGS})

