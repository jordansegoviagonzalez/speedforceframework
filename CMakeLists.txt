cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

# Read version from file
file(READ "VERSION" SPEEDFORCE_VERSION)
string(STRIP "${SPEEDFORCE_VERSION}" SPEEDFORCE_VERSION)

project(SpeedForce VERSION ${SPEEDFORCE_VERSION} LANGUAGES CXX)

# Optional accelerator backends (OFF by default so a clean build works everywhere).
option(SPEEDFORCE_ENABLE_TENSORRT "Build TensorRT backend" OFF)
option(SPEEDFORCE_ENABLE_ONNX     "Build ONNX Runtime backend" OFF)
option(SPEEDFORCE_ENABLE_ROCM     "Build ROCm backend" OFF)

option(SPEEDFORCE_BUILD_SERVER "Build the standalone gRPC server binary" OFF)
option(SPEEDFORCE_BUILD_TESTS  "Build C++ unit tests (gtest required)" OFF)

# Default to C++17 for dependencies (gRPC compatibility)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Performance Optimizations ---
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    add_compile_options(/O2 /Oi /Ot)
else()
    add_compile_options(-O3 -funroll-loops)
endif()

# Pass version to code
add_compile_definitions(SPEEDFORCE_VERSION="${SPEEDFORCE_VERSION}")

# Configure include directories
include_directories(
    "${CMAKE_SOURCE_DIR}/cpp/include"
    "${CMAKE_SOURCE_DIR}/cpp/include/speedforce"
)

# --- Dependency Management ---

include(FetchContent)
enable_testing()

# 3. gRPC dependencies
# Manually fetch Abseil to patch the "unsupported option -msse4.1" error on macOS ARM64
FetchContent_Declare(
    absl
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG        20240116.2
    CMAKE_ARGS     -DABSL_ENABLE_INSTALL=OFF
                   -DABSL_PROPAGATE_CXX_STD=ON
)
FetchContent_MakeAvailable(absl)

set(gRPC_INSTALL OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# Fix for ZLib on macOS: Force use of system zlib to avoid conflict
set(gRPC_ZLIB_PROVIDER "package" CACHE STRING "" FORCE) 

set(ABSL_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(protobuf_INSTALL OFF CACHE BOOL "" FORCE)
set(utf8_range_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(gRPC_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    grpc
    GIT_REPOSITORY https://github.com/grpc/grpc.git
    GIT_TAG        v1.66.0
    CMAKE_ARGS     -DGRPC_BUILD_TESTS=OFF
                   -DgRPC_BENCHMARK_PROVIDER=none
                   -DgRPC_INSTALL=OFF
                   -DgRPC_ZLIB_PROVIDER=package # Use system zlib
                   -DgRPC_PROTOBUF_PROVIDER=package
                   -DgRPC_ABSL_PROVIDER=package
                   -DgRPC_SSL_PROVIDER=package
)
FetchContent_MakeAvailable(grpc)

# ONNX Runtime
FetchContent_Declare(
    onnxruntime
    GIT_REPOSITORY https://github.com/microsoft/onnxruntime.git
    GIT_TAG        v1.16.3
    CMAKE_ARGS     -Donnxruntime_BUILD_SHARED_LIB=OFF -Donnxruntime_BUILD_UNIT_TESTS=OFF -Donnxruntime_BUILD_SAMPLES=OFF -Donnxruntime_BUILD_BENCHMARKS=OFF
)
FetchContent_MakeAvailable(onnxruntime)

# --- Protobuf and gRPC Code Generation ---
set(PROTO_DIR ${CMAKE_SOURCE_DIR}/cpp/server)
set(GENERATED_PROTO_DIR ${CMAKE_BINARY_DIR}/cpp/server/generated)
file(MAKE_DIRECTORY ${GENERATED_PROTO_DIR})

# Use the targets built by FetchContent
# Note: FetchContent usually creates targets like 'libprotobuf' and 'grpc_cpp_plugin'
# We use $<TARGET_FILE:...> to get paths
set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)

set(PROTO_SRCS
    ${GENERATED_PROTO_DIR}/service.pb.cc
    ${GENERATED_PROTO_DIR}/service.grpc.pb.cc
)
set(PROTO_HDRS
    ${GENERATED_PROTO_DIR}/service.pb.h
    ${GENERATED_PROTO_DIR}/service.grpc.pb.h
)

add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS}
    COMMAND ${_PROTOBUF_PROTOC}
        --proto_path=${PROTO_DIR}
        --cpp_out=${GENERATED_PROTO_DIR}
        --grpc_out=${GENERATED_PROTO_DIR}
        --plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN_EXECUTABLE}
        ${PROTO_DIR}/service.proto
    DEPENDS ${PROTO_DIR}/service.proto protoc grpc_cpp_plugin
)

add_library(speedforce_server_proto STATIC
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)
target_link_libraries(speedforce_server_proto
  PRIVATE
    grpc++
    libprotobuf
)
target_include_directories(speedforce_server_proto
  PUBLIC
    ${GENERATED_PROTO_DIR}
)


# --- Core Library Definition ---
add_library(speedforce STATIC
    cpp/src/engine.cpp
    cpp/src/model.cpp
    cpp/src/tensor.cpp
    cpp/src/scheduler.cpp
)

# Conditionally compile optional backends.
if(SPEEDFORCE_ENABLE_TENSORRT)
  target_sources(speedforce PRIVATE cpp/backends/tensorrt/trt_backend.cpp)
  target_compile_definitions(speedforce PUBLIC SPEEDFORCE_ENABLE_TENSORRT=1)
endif()

if(SPEEDFORCE_ENABLE_ONNX)
  target_sources(speedforce PRIVATE cpp/backends/onnxruntime/ort_backend.cpp)
  target_compile_definitions(speedforce PUBLIC SPEEDFORCE_ENABLE_ONNX=1)
endif()

if(SPEEDFORCE_ENABLE_ROCM)
  target_sources(speedforce PRIVATE cpp/backends/rocm/rocm_backend.cpp)
  target_compile_definitions(speedforce PUBLIC SPEEDFORCE_ENABLE_ROCM=1)
endif()

# Link dependencies to the core library
target_link_libraries(speedforce PUBLIC
    speedforce_server_proto
)

if(SPEEDFORCE_ENABLE_ONNX)
    target_link_libraries(speedforce PUBLIC onnxruntime_session)
endif()
target_compile_features(speedforce PRIVATE cxx_std_20)

# --- Main Executable ---
if(SPEEDFORCE_BUILD_SERVER)
    add_executable(speedforce_server
        cpp/server/server.cpp
        cpp/server/service_impl.h
    )
    target_link_libraries(speedforce_server PRIVATE
        speedforce
        grpc++      # Direct target name
        grpc        # Direct target name
        libprotobuf # Direct target name
    )
endif()


# --- C++ Tests ---
# The gRPC FetchContent above also fetches GoogleTest. We can use the `gtest_main` target directly.
if(SPEEDFORCE_BUILD_TESTS)
    add_executable(speedforce_test_engine cpp/tests/test_engine.cpp)
    target_link_libraries(speedforce_test_engine PRIVATE gtest_main speedforce)

    add_executable(speedforce_test_scheduler cpp/tests/test_scheduler.cpp)
    target_link_libraries(speedforce_test_scheduler PRIVATE gtest_main speedforce)

    add_test(NAME EngineTests COMMAND speedforce_test_engine)
    add_test(NAME SchedulerTests COMMAND speedforce_test_scheduler)
endif()


# --- Python Bindings ---
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Required for linking static lib into shared python module

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG        v2.11.1
)
FetchContent_MakeAvailable(pybind11)

pybind11_add_module(_bindings
    python/src/speedforce/_internal/bindings.cpp
)

# Link against the core library
target_link_libraries(_bindings PRIVATE speedforce)

# Copy the generated extension to the source tree so Python can find it easily during dev
add_custom_command(TARGET _bindings POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:_bindings> "${CMAKE_SOURCE_DIR}/python/src/speedforce/_internal/"
)
