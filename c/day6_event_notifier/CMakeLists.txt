cmake_minimum_required(VERSION 3.8)

project(c-event-notifier)

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_C_FLAGS "-Wall")

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
include_directories(${COMMON_INCLUDES})
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

add_library(c-event-notifier ${SRC_FILES})
target_compile_features(c-event-notifier PUBLIC cxx_std_20)
target_compile_features(c-event-notifier PUBLIC c_std_11)


find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
enable_testing()

find_package (Threads REQUIRED)

# Include the gtest library. gtest_SOURCE_DIR is available due to
# 'project(gtest)' above.
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

##############
# Unit Tests
##############
add_executable(runUnitTests ${TEST_SRC_FILES})
target_link_libraries(runUnitTests gtest gtest_main pthread)
target_link_libraries(runUnitTests c-event-notifier pthread)
target_link_options(runUnitTests PRIVATE "LINKER:--wrap=malloc")
target_link_options(runUnitTests PRIVATE "LINKER:--wrap=calloc")
target_link_options(runUnitTests PRIVATE "LINKER:--wrap=realloc")
target_link_options(runUnitTests PRIVATE "LINKER:--wrap=free")
add_test(UnitTests runUnitTests)
