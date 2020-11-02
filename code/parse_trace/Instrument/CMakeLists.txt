add_llvm_library(Instrument MODULE 
    InjectFuncCall.cpp
    InjectFuncCall.h
    PLUGIN_TOOL
    opt 
)


#require c++14 at least
set_property(TARGET Instrument PROPERTY CXX_STANDARD 14)
set_property(TARGET Instrument PROPERTY CXX_STANDARD_REQUIRED ON)
