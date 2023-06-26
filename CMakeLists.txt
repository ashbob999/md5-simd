cmake_minimum_required(VERSION 3.1)

project(md5-simd)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DUSE_256_BITS)

add_library(md5-original source/original/md5-original.cpp)
target_include_directories(md5-original PUBLIC source/original)

add_library(md5-simd source/simd/md5-simd.cpp)
target_include_directories(md5-simd PUBLIC source/simd)
if (MSVC)
  target_compile_options(md5-simd PUBLIC "/arch:AVX2")
else()
  target_compile_options(md5-simd PUBLIC "-mavx2")
endif()

add_executable(md5-test test.cpp)
target_link_libraries(md5-test md5-original md5-simd)
