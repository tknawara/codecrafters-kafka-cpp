cmake_minimum_required(VERSION 3.13)

project(codecrafters-kafka)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(fmt CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(Backward CONFIG REQUIRED)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS  src/*.cpp src/*.hpp)

# 1. Find the clang-tidy executable on your system
find_program(CLANG_TIDY_EXE NAMES "clang-tidy-19" "clang-tidy")

if(CLANG_TIDY_EXE)
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
    
    # 2. Set the global CMake property to run it on all C++ targets
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
else()
    message(WARNING "clang-tidy not found! Static analysis disabled.")
endif()

add_executable(kafka ${SOURCE_FILES})

target_include_directories(kafka PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(kafka 
  PRIVATE 
  fmt::fmt 
  asio::asio
  Backward::Interface)

target_compile_options(kafka PRIVATE -Wall -Wextra -Werror=return-type -Wpedantic -Wshadow -Wnon-virtual-dtor)
target_link_options(kafka PRIVATE -rdynamic)