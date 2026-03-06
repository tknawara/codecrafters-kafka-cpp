cmake_minimum_required(VERSION 3.13)

project(codecrafters-kafka)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor)

find_package(fmt CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS  src/*.cpp src/*.hpp)

add_executable(kafka ${SOURCE_FILES})

target_include_directories(kafka PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(kafka 
  PRIVATE 
  fmt::fmt 
  asio::asio)