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

add_subdirectory(server)
