cmake_minimum_required(VERSION 3.15)
project(chat_engine)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

set(LINK_SHARED_ZED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# ZED SDK Options
# option(LINK_SHARED_ZED "Link with the ZED SDK shared executable" ON)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)
find_package(ZED REQUIRED)
find_package(CUDA ${ZED_CUDA_VERSION} REQUIRED)
find_package(quill REQUIRED)

# Find libdatachannel
find_library(DATACHANNEL_LIB datachannel REQUIRED)
if(NOT DATACHANNEL_LIB)
    message(FATAL_ERROR "libdatachannel not found")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utils/async_logger/include)
include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${ZED_INCLUDE_DIRS})
include_directories(${OPENCV_INCLUDE_DIRS})

link_directories(${ZED_LIBRARY_DIR})
link_directories(${CUDA_LIBRARY_DIRS})

# aux_source_directory(src SRC_LIST)
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/proto/*.proto")
file(GLOB_RECURSE SERVER_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/server/*.cpp")
file(GLOB_RECURSE CLIENT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/client/*.cpp")

# PROTOBUF GENERATION
# Build chatproto as a shared library to speed up linking
add_library(chatproto SHARED ${PROTO_FILES})
set_target_properties(chatproto PROPERTIES
    # Where to place the resulting libchatproto.so
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib/proto"
    # Keep archive dir set as well for platforms/generators that may still emit an import/archive
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

add_executable(chat_server
    ${SERVER_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/server_main.cpp)
add_executable(chat_client
    ${CLIENT_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/client_main.cpp)

# Use static or dynamic library for ZED
if(LINK_SHARED_ZED)
    SET(ZED_LIBS ${ZED_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY})
else()
    SET(ZED_LIBS ${ZED_STATIC_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_LIBRARY})
endif()


target_link_libraries(chat_server chatproto)
target_link_libraries(chat_server ${DATACHANNEL_LIB})
target_link_libraries(chat_server Threads::Threads)
target_link_libraries(chat_server quill::quill)

target_link_libraries(chat_client chatproto)
target_link_libraries(chat_client Threads::Threads)
target_link_libraries(chat_client quill::quill)
target_link_libraries(chat_client ${ZED_LIBS})
target_link_libraries(chat_client ${OpenCV_LIBS})
target_link_libraries(chat_client ${ZED_LIBS})
target_link_libraries(chat_client ${DATACHANNEL_LIB})

# Ensure executables can find libchatproto.so at runtime when running from the build tree
set_target_properties(chat_client chat_server PROPERTIES
    BUILD_RPATH "\$ORIGIN/../lib;\$ORIGIN/../lib/proto"
)
