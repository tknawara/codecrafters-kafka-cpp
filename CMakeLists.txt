cmake_minimum_required(VERSION 3.13)

project(codecrafters-kafka)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
# Enforce C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor)
find_package(fmt CONFIG REQUIRED)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

add_executable(kafka ${SOURCE_FILES})
target_link_libraries(kafka PRIVATE fmt::fmt)