cmake_minimum_required(VERSION 3.20)

project(MiniRedis LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(miniredis
    src/main.cpp
    src/utils/utils.cpp
    src/storage/storage.cpp
    src/server/server.cpp
    src/client/client.cpp
    src/protocol/message.cpp
)