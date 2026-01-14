# CMake minimum version requirement
cmake_minimum_required(VERSION 3.10)

# Project name
project(ExplicitKeywordExample)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(explicit_keyword_example main.cpp)

# Optional: Add compiler warnings for good practice
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()
