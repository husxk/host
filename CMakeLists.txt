cmake_minimum_required(VERSION 3.20)

project(host
    VERSION 0.1.0
    DESCRIPTION "NAS server"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_path(RAPIDJSON_INCLUDE_DIR rapidjson/document.h REQUIRED)

option(HOST_BUILD_TESTS "Build tests" ON)
option(HOST_COVERAGE "Build with coverage instrumentation" OFF)

if(HOST_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

if(HOST_BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(server)
add_subdirectory(cli)
