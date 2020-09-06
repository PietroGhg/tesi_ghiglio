cmake_minimum_required(VERSION 3.4.3)
project(map)

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

llvm_map_components_to_libnames(llvm_libs
  core
  mc
  debuginfocodeview
  object
  debuginfomsf
  binaryformat
  support
  demangle
  debuginfodwarf
  ${LLVM_TARGETS_TO_BUILD} #for InitAllTargets
)



add_executable(map
  map.cpp
)

target_link_libraries(map ${llvm_libs})
target_compile_options(map PRIVATE -g)
set_property(TARGET map PROPERTY CXX_STANDARD 14)
set_property(TARGET map PROPERTY CXX_STANDARD_REQUIRED ON)
