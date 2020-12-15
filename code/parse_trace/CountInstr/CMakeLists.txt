add_llvm_library(CountBB MODULE 
    CountBB.cpp
    CountBB.h
    PLUGIN_TOOL
    opt 
)


#require c++14 at least
set_property(TARGET CountBB PROPERTY CXX_STANDARD 14)
set_property(TARGET CountBB PROPERTY CXX_STANDARD_REQUIRED ON)
