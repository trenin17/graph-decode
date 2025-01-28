cmake_minimum_required(VERSION 3.20)
project(MyBoostFibersProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include Conan-generated files
#include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

# Add subdirectory for source files
add_subdirectory(src)
