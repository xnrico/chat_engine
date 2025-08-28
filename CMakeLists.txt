cmake_minimum_required(VERSION 3.15)
project(chat_engine)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_STANDARD 17)

set(RPC_INCLUDE_DIR "/usr/local/include/rpc")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(RPCLIB REQUIRED rpclib)

include_directories(${RPC_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

aux_source_directory(src SRC_LIST)

add_executable(chat_engine ${SRC_LIST})

target_link_libraries(chat_engine rpc)
target_link_libraries(chat_engine Threads::Threads)