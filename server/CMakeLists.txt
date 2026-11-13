set(APP_NAME host)

file(GLOB_RECURSE HOST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(${APP_NAME}
    main.cpp
    ${HOST_SOURCES}
)

target_include_directories(${APP_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${RAPIDJSON_INCLUDE_DIR}
)

target_compile_options(${APP_NAME}
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)
