cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)
find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(include)
link_directories(${LLVM_LIBRARY_DIRS})

add_subdirectory(src)  # Use your pass name here.
