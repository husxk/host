set(CLI_APP_NAME host-cli)

file(GLOB_RECURSE CLI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(${CLI_APP_NAME}
    main.cpp
    ${CLI_SOURCES}
)

target_include_directories(${CLI_APP_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${CLI_APP_NAME}
    PRIVATE
        host_common
)
