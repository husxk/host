cmake_minimum_required(VERSION 3.20)

project(host
    VERSION 0.1.0
    DESCRIPTION "NAS server"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(APP_NAME host)

find_package(RapidJSON REQUIRED)

file(GLOB_RECURSE HOST_SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_executable(${APP_NAME}
    main.cpp
    ${HOST_SOURCES}
)

target_include_directories(${APP_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${APP_NAME}
    PRIVATE
        RapidJSON
)

target_compile_options(${APP_NAME}
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)
