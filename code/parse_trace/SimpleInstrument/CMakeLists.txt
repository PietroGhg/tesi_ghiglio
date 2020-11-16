add_llvm_library(SimpleInstrument MODULE 
    SimpleInject.cpp
    SimpleInject.h
    PLUGIN_TOOL
    opt 
)


#require c++14 at least
set_property(TARGET SimpleInstrument PROPERTY CXX_STANDARD 14)
set_property(TARGET SimpleInstrument PROPERTY CXX_STANDARD_REQUIRED ON)
