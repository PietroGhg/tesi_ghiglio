#include "SimpleInject.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

PreservedAnalyses SimpleInject::run(llvm::Module &M,
                                       llvm::ModuleAnalysisManager &) {
  bool Changed =  runOnModule(M);

  return (Changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

bool LegacySimpleInject::runOnModule(llvm::Module &M) {
  bool Changed = Impl.runOnModule(M);

  return Changed;
}

bool SimpleInject::runOnModule(Module& M){
  auto& CTX = M.getContext();
  unsigned long bbCount = 0;
  IRBuilder<> Builder(CTX);

  //definition of printf
  PointerType *PrintfArgTy = PointerType::getUnqual(Type::getInt8Ty(CTX));


  FunctionType *PrintfTy = FunctionType::get(
      IntegerType::getInt32Ty(CTX),
      PrintfArgTy,
      true); //isVarArgs


  FunctionCallee Printf = M.getOrInsertFunction("printf", PrintfTy);

  Function *PrintfF = dyn_cast<Function>(Printf.getCallee());
  PrintfF->setDoesNotThrow();
  PrintfF->addParamAttr(0, Attribute::NoCapture);
  PrintfF->addParamAttr(0, Attribute::ReadOnly);

  //format string for the basick block id
  llvm::Constant *bb_id_str = llvm::ConstantDataArray::getString(
      CTX, "%d\n");
  Constant *bb_id_str_var =
      M.getOrInsertGlobal("bb_id_str", bb_id_str->getType());
  dyn_cast<GlobalVariable>(bb_id_str_var)->setInitializer(bb_id_str);

  for(auto& F : M){
    for(auto& BB : F){
      //print the id of the module
      Builder.SetInsertPoint(&*(BB.getFirstInsertionPt()));
        //print basick block id
      llvm::Value *bb_id_ptr =
	Builder.CreatePointerCast(bb_id_str_var, PrintfArgTy, "formatStr");
      Builder.CreateCall(PrintfTy, PrintfF,
			 {bb_id_ptr, Builder.getInt32(bbCount)}, "print_bb_id");
      bbCount++;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getSimpleInjectPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "simple-inject", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "simple-inject") {
                    MPM.addPass(SimpleInject());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getSimpleInjectPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacySimpleInject::ID = 0;

// Register the pass - required for (among others) opt
static RegisterPass<LegacySimpleInject>
    X(/*PassArg=*/"legacy-simple-inject", /*Name=*/"LegacySimpleInject",
      /*CFGOnly=*/false, /*is_analysis=*/false);
