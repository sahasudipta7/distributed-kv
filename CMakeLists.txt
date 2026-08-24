cmake_minimum_required(VERSION 3.15)
project(distributed_kv)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main executable
add_executable(distributed_kv
    src/main.cpp
    src/kv_store.cpp
    src/command.cpp
    src/wal.cpp
)

# ---- Tests ----
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Prevent GTest from overriding parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(kv_tests
    tests/kv_store_test.cpp
    src/kv_store.cpp
    src/command.cpp
    src/wal.cpp
)
target_link_libraries(kv_tests gtest_main)

include(GoogleTest)
gtest_discover_tests(kv_tests)