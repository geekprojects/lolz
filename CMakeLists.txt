cmake_minimum_required(VERSION 3.10)
project(lolz VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)

pkg_check_modules(libgeek REQUIRED libgeek)
pkg_check_modules(yamlcpp REQUIRED yaml-cpp)

find_library(fsw_LIBRARY NAMES fswatch )
find_path(fsw_INCLUDE_BASE NAMES libfswatch/c++/monitor_factory.hpp)

include_directories(
    ${fsw_INCLUDE_BASE}
)

add_definitions(${libgeek_CFLAGS} ${yamlcpp_CFLAGS})

add_executable(lolz
    src/main.cpp
    src/lolz.cpp
    src/logdirectory.cpp
    src/logfile.cpp
)

target_link_libraries(lolz ${libgeek_LDFLAGS} ${fsw_LIBRARY} ${yamlcpp_LDFLAGS})

