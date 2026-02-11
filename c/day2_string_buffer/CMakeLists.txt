cmake_minimum_required(VERSION 3.10)
project(day2_string_buffer)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_library(strbuf
    src/strbuf.c
)

target_include_directories(strbuf PUBLIC src)

target_compile_options(strbuf PRIVATE -Wall -Wextra -Werror)

add_executable(strbuf_tests
    tests/strbuf_test.cpp
)

target_link_libraries(strbuf_tests
    strbuf
    gtest
    gtest_main
    pthread
)

add_test(NAME strbuf_tests COMMAND strbuf_tests)
