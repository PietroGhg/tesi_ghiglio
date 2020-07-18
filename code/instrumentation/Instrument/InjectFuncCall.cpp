#include "InjectFuncCall.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

#include "llvm/IR/GlobalValue.h" //global variable linkage types

using namespace llvm;

#define DEBUG_TYPE "inject-func-call"

//-----------------------------------------------------------------------------
// InjectFuncCall implementation
//-----------------------------------------------------------------------------
bool InjectFuncCall::runOnModule(Module &M) {
  auto &CTX = M.getContext();

  //define struct type
  auto s_type = StructType::create(CTX, "s_stack");
  ArrayRef<Type*> elements = {IntegerType::getInt32Ty(CTX), s_type};
  s_type->setBody(elements, true);

  //inject global variable
  
  IRBuilder<> builder(CTX);
  Instruction *I = &*inst_begin(M.getFunction("main"));
  builder.SetInsertPoint(I);
  M.getOrInsertGlobal("globalKey", builder.getInt64Ty());
  GlobalVariable* g_var = M.getNamedGlobal("globalKey");
  g_var->setLinkage(GlobalValue::InternalLinkage);
  g_var->setAlignment(Align(8));
  g_var->setInitializer(builder.getInt64(69));
  g_var->setConstant(false);

  bool injectedAtLeastOnce = false;


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

  llvm::Constant *PrintfFormatStr = llvm::ConstantDataArray::getString(
      CTX, "(llvm-tutor) Hello from: %s\n(llvm-tutor)   called at line: %d   global: %d\n");

  Constant *PrintfFormatStrVar =
      M.getOrInsertGlobal("PrintfFormatStr", PrintfFormatStr->getType());
  dyn_cast<GlobalVariable>(PrintfFormatStrVar)->setInitializer(PrintfFormatStr);


  for (auto &F : M) {
    if (F.isDeclaration())
      continue;

    for(auto& bb : F){
      for(auto& i : bb){
        if(isa<CallInst>(i) && i.getDebugLoc() && !isa<DbgInfoIntrinsic>(i)){
          auto line = i.getDebugLoc().getLine();
          
          //IRBuilder<> builder(&i);
          builder.SetInsertPoint(&i);
          auto i_call = dyn_cast<CallInst>(&i);
          auto name_str = i_call->getCalledFunction()->getName();
          auto FuncName = builder.CreateGlobalStringPtr(i_call->getCalledFunction()->getName());
          llvm::Value *FormatStrPtr =
            builder.CreatePointerCast(PrintfFormatStrVar, PrintfArgTy, "formatStr");
          
          //load global variable
          auto load = builder.CreateLoad(g_var, "load_g_var");
          builder.CreateCall(
            Printf, {FormatStrPtr, FuncName, builder.getInt32(line), load});
          injectedAtLeastOnce = true;
        }
      }
    }
  }

  return injectedAtLeastOnce;
}

PreservedAnalyses InjectFuncCall::run(llvm::Module &M,
                                       llvm::ModuleAnalysisManager &) {
  bool Changed =  runOnModule(M);

  return (Changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

bool LegacyInjectFuncCall::runOnModule(llvm::Module &M) {
  bool Changed = Impl.runOnModule(M);

  return Changed;
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getInjectFuncCallPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "inject-func-call", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "inject-func-call") {
                    MPM.addPass(InjectFuncCall());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getInjectFuncCallPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyInjectFuncCall::ID = 0;

// Register the pass - required for (among others) opt
static RegisterPass<LegacyInjectFuncCall>
    X(/*PassArg=*/"legacy-inject-func-call", /*Name=*/"LegacyInjectFuncCall",
      /*CFGOnly=*/false, /*is_analysis=*/false);
