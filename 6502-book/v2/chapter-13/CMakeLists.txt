cmake_minimum_required(VERSION 3.10)
project(chapter-13)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-Wno-write-strings)

find_package(SDL2 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

include_directories(${CMAKE_SOURCE_DIR}/inc ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

# Source files
file(GLOB SRCS "src/*.cpp")

add_executable(chapter-13 ${SRCS})

target_link_libraries(chapter-13 ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
