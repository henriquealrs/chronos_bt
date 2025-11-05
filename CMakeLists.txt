cmake_minimum_required(VERSION 3.20)

project(chronos VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
find_package(ZeroMQ REQUIRED)
find_package(RapidJSON REQUIRED)

add_subdirectory(core)
enable_testing()
add_subdirectory(tests)
