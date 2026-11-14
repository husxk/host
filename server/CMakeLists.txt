set(APP_NAME host)
set(CORE_NAME host_core)

file(GLOB_RECURSE HOST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(${CORE_NAME} STATIC
    ${HOST_SOURCES}
)

target_include_directories(${CORE_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${RAPIDJSON_INCLUDE_DIR}
)

target_compile_options(${CORE_NAME}
    PUBLIC
        -Wall
        -Wextra
        -Wpedantic
)

add_executable(${APP_NAME}
    main.cpp
)

target_link_libraries(${APP_NAME}
    PRIVATE
        ${CORE_NAME}
)

if(HOST_BUILD_TESTS)
    add_subdirectory(tests)
endif()
