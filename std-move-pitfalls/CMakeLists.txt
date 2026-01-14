cmake_minimum_required(VERSION 3.10)

project(StdMovePitfallExample)

# Set the C++ standard to C++11 or higher for std::move
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(std_move_pitfall_example main.cpp)

