cmake_minimum_required(VERSION 3.4.3)
project(parseTrace)

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

llvm_map_components_to_libnames(
    llvm_libs
    Core
    IRReader
)



add_executable(parsetrace
  parseTrace.cpp
)

target_link_libraries(parsetrace ${llvm_libs})
target_compile_options(parsetrace PRIVATE -g)
set_property(TARGET parsetrace PROPERTY CXX_STANDARD 14)
set_property(TARGET parsetrace PROPERTY CXX_STANDARD_REQUIRED ON)