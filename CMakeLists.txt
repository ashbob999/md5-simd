cmake_minimum_required(VERSION 3.1)

project(md5-simd)

set(CMAKE_CXX_STANDARD 17)

add_executable(md5-simd "source/simd/md5-simd.cpp" "source/original/md5-original.cpp" "test.cpp")
