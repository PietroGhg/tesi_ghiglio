add_llvm_library(AddIRDebug MODULE 
    AddIRDebug.cpp  
    PLUGIN_TOOL
    opt 
)


#require c++14 at least
set_property(TARGET AddIRDebug PROPERTY CXX_STANDARD 14)
set_property(TARGET AddIRDebug PROPERTY CXX_STANDARD_REQUIRED ON)