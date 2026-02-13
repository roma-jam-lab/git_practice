cmake_minimum_required(VERSION 3.10)
project(day5_dlist)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_library(dlist
    src/dlist.c
)

target_include_directories(dlist PUBLIC src)

target_compile_options(dlist PRIVATE -Wall -Wextra -Werror)

add_executable(dlist_tests
    tests/dlist_tests.cpp
)

target_link_libraries(dlist_tests
    dlist
    gtest
    gtest_main
    pthread
)

add_test(NAME dlist_tests COMMAND dlist_tests)
