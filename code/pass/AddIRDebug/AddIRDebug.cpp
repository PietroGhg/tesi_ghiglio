//opt -S -load=./AddIRDebug.so -addIRdbg test.ll 
//Module pass that produces a .ll file such that the line debug info is replaced with its own line number.
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/MDBuilder.h"

using namespace llvm;

namespace {
struct AddIRDebug : public ModulePass {
  static char ID;
  AddIRDebug() : ModulePass(ID) {}

  MDNode* oldScope;
  DILocation* oldLoc;
  bool oldImplicit;
  DebugLoc new_loc;
  DebugLoc curr_loc;
  MDNode* scope;
  DILocation* inlinedAt;
  bool isImpilicitCode;
  
  bool runOnModule(Module& M) override {
    unsigned line_count = 0;
    for(auto& f : M.getFunctionList()){  
      errs() << f.getName() << "\n";

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
        errs() << bb.getName() << "\n";
        for(auto& i : bb.getInstList()){
          if(auto loc = i.getDebugLoc()){
	    curr_loc = loc;                 
          }
	  new_loc = DebugLoc::get(line_count, 0,
				  curr_loc.getScope(),
				  curr_loc.getInlinedAt(),
				  curr_loc.isImplicitCode());
	  i.setDebugLoc(new_loc);   
	  errs() << i << " " << i.getDebugLoc().getLine() << "\n"; 
          line_count++;
         }
        }
    }

    return false;
  }
}; 
}  

char AddIRDebug::ID = 0;
static RegisterPass<AddIRDebug> X("addIRdbg", "Adds IR line to debug info",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);


static RegisterStandardPasses Y(
    PassManagerBuilder::EP_EarlyAsPossible,
    [](const PassManagerBuilder &Builder,
       legacy::PassManagerBase &PM) { PM.add(new AddIRDebug()); });
