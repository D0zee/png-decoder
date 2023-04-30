project(png-decoder)
cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD             17)

find_package(PNG)

if (NOT DEFINED PNG_FOUND)
    message(FATAL_ERROR "Can't find libpng")
endif()

include(build.cmake)