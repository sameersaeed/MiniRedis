cmake_minimum_required(VERSION 3.20)

project(MiniRedis LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(miniredis
    src/main.cpp
    src/utils/utils.cpp
    src/storage/storage.cpp
    src/server/server.cpp
    src/client/client.cpp
    src/protocol/message.cpp
    src/concurrency/threadpool.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(miniredis PRIVATE Threads::Threads)

###############################################################################

# for tests

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(miniredis_tests
    tests/storage_tests.cpp
    tests/parser_tests.cpp
    tests/executor_tests.cpp
    tests/integration_tests.cpp
    src/utils/utils.cpp
    src/storage/storage.cpp
    src/server/server.cpp
    src/client/client.cpp
    src/protocol/message.cpp
    src/concurrency/threadpool.cpp
)

target_include_directories(miniredis_tests PRIVATE src)
target_link_libraries(miniredis_tests PRIVATE GTest::gtest_main Threads::Threads)

include(GoogleTest)
gtest_discover_tests(miniredis_tests)