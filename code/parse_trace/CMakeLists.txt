cmake_minimum_required(VERSION 3.4.3)
project(parseTrace)

find_package(LLVM REQUIRED CONFIG)

SET(CMAKE_CXX_COMPILER /home/pietro/llvm/bin/clang++)
#SET(CMAKE_CXX_FLAGS "-fsanitize=address -fno-omit-frame-pointer")

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

llvm_map_components_to_libnames(llvm_libs
  irreader
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

include_directories(include)
add_subdirectory(Instrument)
add_subdirectory(SimpleInstrument)
add_subdirectory(AddIRDebug)
add_subdirectory(CountInstr)
add_executable(parsetrace
  parseTrace.cpp
  sourcelocation.cpp
  extendCall.cpp
  costmap.cpp
)


target_link_libraries(parsetrace ${llvm_libs} jsoncpp)
target_compile_options(parsetrace PUBLIC -O2 -g -fno-rtti)
set_property(TARGET parsetrace PROPERTY CXX_STANDARD 14)
set_property(TARGET parsetrace PROPERTY CXX_STANDARD_REQUIRED ON)
