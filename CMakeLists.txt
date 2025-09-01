cmake_minimum_required(VERSION 3.15)
project(chat_engine)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

aux_source_directory(src SRC_LIST)
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/*.proto")

# PROTOBUF GENERATION
add_library(chatproto ${PROTO_FILES})
set_target_properties(chatproto PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/proto"
)

target_link_libraries(chatproto
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)

target_include_directories(chatproto PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/proto")

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(
    TARGET chatproto
    LANGUAGE cpp
    PROTOC_OUT_DIR "${CMAKE_SOURCE_DIR}/include/proto"
    IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/proto"
)
protobuf_generate(
    TARGET chatproto 
    LANGUAGE grpc 
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc 
    PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}" 
    PROTOC_OUT_DIR "${CMAKE_SOURCE_DIR}/include/grpc"
    IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/proto"
)

add_executable(chat_engine ${SRC_LIST})
target_link_libraries(chat_engine chatproto)
target_link_libraries(chat_engine Threads::Threads)