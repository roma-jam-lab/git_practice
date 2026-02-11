cmake_minimum_required(VERSION 3.10)
project(day1_dynamic_array)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_library(dyn_array
    src/dyn_array.c
)

target_include_directories(dyn_array PUBLIC src)

target_compile_options(dyn_array PRIVATE -Wall -Wextra -Werror)

add_executable(dyn_array_tests
    tests/dyn_array_test.cpp
)

target_link_libraries(dyn_array_tests
    dyn_array
    gtest
    gtest_main
    pthread
)

add_test(NAME dyn_array_tests COMMAND dyn_array_tests)