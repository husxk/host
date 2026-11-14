set(TESTS_NAME host_tests)

find_package(GTest REQUIRED)
include(GoogleTest)

file(GLOB_RECURSE TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(${TESTS_NAME}
    ${TEST_SOURCES}
)

target_link_libraries(${TESTS_NAME}
    PRIVATE
        host_core
        GTest::gtest_main
)

gtest_discover_tests(${TESTS_NAME})
