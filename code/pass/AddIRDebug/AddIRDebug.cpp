//opt -S -load=./AddIRDebug.so -addIRdbg test.ll 
//Module pass that produces a .ll file such that the line debug info is replaced with its own line number.
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/MDBuilder.h"

#include "llvm/IR/IRBuilder.h"



using namespace llvm;

namespace {
struct AddIRDebug : public ModulePass {
  static char ID;
  AddIRDebug() : ModulePass(ID) {}

  DebugLoc new_loc;
  DebugLoc curr_loc;
  MDNode* scope;
  DILocation* inlinedAt;
  bool isImpilicitCode;
  
  bool runOnModule(Module& M) override {
    unsigned line_count = 1;
    for(auto& f : M.getFunctionList()){
      if(f.isDeclaration()){
	continue;
      }
      //add dummy instruction at beginning of function and set debug location of function to that instruction.
      auto& i = *(f.begin()->begin());
      auto& ctx = M.getContext();
      IRBuilder<> builder(ctx);
      builder.SetInsertPoint(&i);
      builder.CreateAlloca(builder.getInt1Ty(), builder.getInt1(0));

      //find the first debugloc;
      for(auto& bb : f){
	for(auto& i : bb){
	  if(auto loc = i.getDebugLoc()){
	    curr_loc = loc;
	    break;
	  }
	}
      }
      
      
      auto oldSubProg = f.getSubprogram();
      auto line = line_count;
      auto newSubProg =
	DISubprogram::getDistinct(ctx,
				  oldSubProg->getScope(),
				  oldSubProg->getName(),
				  oldSubProg->getLinkageName(),
				  oldSubProg->getFile(),
				  line,//line
				  oldSubProg->getType(),
				  line, //oldSubProg->getScopeLine(), //??
				  oldSubProg->getContainingType(),
				  oldSubProg->getVirtualIndex(),
				  oldSubProg->getThisAdjustment(),
				  oldSubProg->getFlags(),
				  oldSubProg->getSPFlags(),
				  oldSubProg->getUnit(),
				  oldSubProg->getTemplateParams(),
				  oldSubProg->getDeclaration(),
				  oldSubProg->getRetainedNodes(),
				  oldSubProg->getThrownTypes());
      f.setSubprogram(newSubProg);

      for(auto& bb : f.getBasicBlockList()){
        for(auto& i : bb.getInstList()){
          if(auto loc = i.getDebugLoc()){
	    curr_loc = loc;                 
          }
	  new_loc = DebugLoc::get(line_count, 0,
				  newSubProg,
				  curr_loc.getInlinedAt(),
				  curr_loc.isImplicitCode());
	  i.setDebugLoc(new_loc);   
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
