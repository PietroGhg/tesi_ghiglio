//opt -S -load=./MyHello.so -hello test.ll 
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {
struct AddIRDebug : public FunctionPass {
  static char ID;
  AddIRDebug() : FunctionPass(ID) {}

  bool runOnFunction(Function& F) override {
    errs() << "Funzione: ";
    errs().write_escaped(F.getName()) <<  " " << F.hasMetadata() << "\n";
    
    
    for(auto& bb : F.getBasicBlockList()){
      for(auto& i : bb.getInstList()){
        errs() << i << " " << i.hasMetadata() << "\n";
      }
    }

    return false;
  }
}; // end of struct Hello
}  // end of anonymous namespace

char AddIRDebug::ID = 0;
static RegisterPass<AddIRDebug> X("addIRdbg", "Adds IR line to debug info",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);


static RegisterStandardPasses Y(
    PassManagerBuilder::EP_EarlyAsPossible,
    [](const PassManagerBuilder &Builder,
       legacy::PassManagerBase &PM) { PM.add(new AddIRDebug()); });
