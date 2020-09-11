#include <memory>
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DebugInfoMetadata.h"

using namespace llvm;

inline std::unique_ptr<Module> replaceDebug(const Module& old_module){
  DebugLoc new_loc;
  DebugLoc curr_loc;

  auto m = CloneModule(old_module);
  if(!m){
    errs() << "cannot clone module\n";
    throw;
  }

  unsigned line_count = 0;
    for(auto& f : m->getFunctionList()){  
      //find the first debugloc;
      for(auto& bb : f){
	for(auto& i : bb){
	  if(auto loc = i.getDebugLoc()){
	    curr_loc = loc;
	    break;
	  }
	}
      }
      
      for(auto& bb : f.getBasicBlockList()){
        for(auto& i : bb.getInstList()){
          if(auto loc = i.getDebugLoc()){
	    curr_loc = loc;                 
          }
	  new_loc = DebugLoc::get(line_count, 0,
				  curr_loc.getScope(),
				  curr_loc.getInlinedAt(),
				  curr_loc.isImplicitCode());
	  i.setDebugLoc(new_loc);   
          line_count++;
         }
        }
    }

    return std::move(m);
}
