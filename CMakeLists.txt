cmake_minimum_required(VERSION 3.10)
project(lolz VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)

pkg_check_modules(libgeek REQUIRED libgeek)
find_library(FSW_LIBRARY NAMES fswatch )

add_definitions(${libgeek_CFLAGS})

add_executable(lolz
    src/main.cpp
    src/lolz.cpp
    src/logdirectory.cpp
    src/logfile.cpp
)

target_link_libraries(lolz ${libgeek_LDFLAGS} ${FSW_LIBRARY})

