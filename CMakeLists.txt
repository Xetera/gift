cmake_minimum_required(VERSION 3.13)
project(gift)
set(CMAKE_CXX_STANDARD 17)

file (GLOB CPP_FILES src/*.cpp)

set(SOURCE_FILES ${CPP_FILES})
include_directories(include)
add_executable(gift ${SOURCE_FILES})