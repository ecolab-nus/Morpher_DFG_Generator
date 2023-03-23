cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 17)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(include)
link_directories(${LLVM_LIBRARY_DIRS})

add_subdirectory(src)  # Use your pass name here.
