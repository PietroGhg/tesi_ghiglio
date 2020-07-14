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

  bool runOnModule(Module& M) override {
    unsigned line_count = 0;
    for(auto& f : M.getFunctionList()){  
      for(auto& bb : f.getBasicBlockList()){
        for(auto& i : bb.getInstList()){
          DebugLoc new_loc;
          if(auto loc = i.getDebugLoc()){
            new_loc = DebugLoc::get(line_count, 0, loc.getScope(), loc.getInlinedAt(), loc.isImplicitCode());
                        
          }
          else{
            new_loc = DebugLoc::get(line_count, 0, nullptr, nullptr, false);       
          }
          i.setMetadata(LLVMContext::MD_dbg, new_loc.get()); //IR/FixedMetadataKinds.def 
          errs() << i << " line: " << line_count << "\n";
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
