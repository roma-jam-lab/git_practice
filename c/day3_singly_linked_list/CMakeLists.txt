cmake_minimum_required(VERSION 3.10)
project(day3_slist)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_library(slist
    src/slist.c
)

target_include_directories(slist PUBLIC src)

target_compile_options(slist PRIVATE -Wall -Wextra -Werror)

add_executable(slist_tests
    tests/slist_test.cpp
    tests/slist_algorithms_test.cpp
)

target_link_libraries(slist_tests
    slist
    gtest
    gtest_main
    pthread
)

add_test(NAME slist_tests COMMAND slist_tests)
