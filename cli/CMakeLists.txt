set(CLI_APP_NAME host-cli)

add_executable(${CLI_APP_NAME}
    main.cpp
)

target_compile_options(${CLI_APP_NAME}
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)
