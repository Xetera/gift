cmake_minimum_required(VERSION 3.13)  # CMake version check
project(gift)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 17)            # Enable c++14 standard

# Add main.cpp file of project root directory as source file
file (GLOB CPP_FILES src/*.cpp)

set(SOURCE_FILES ${CPP_FILES})
include_directories(include)
# Add executable target with source files listed in SOURCE_FILES variable
add_executable(gift ${SOURCE_FILES})