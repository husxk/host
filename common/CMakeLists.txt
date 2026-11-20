set(COMMON_NAME host_common)

file(GLOB_RECURSE COMMON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(${COMMON_NAME} STATIC
    ${COMMON_SOURCES}
)

target_include_directories(${COMMON_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${RAPIDJSON_INCLUDE_DIR}
)

target_compile_options(${COMMON_NAME}
    PUBLIC
        -Wall
        -Wextra
        -Wpedantic
)
