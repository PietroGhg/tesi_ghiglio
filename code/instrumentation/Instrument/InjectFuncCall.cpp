#include "InjectFuncCall.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

#include "llvm/IR/GlobalValue.h" //global variable linkage types
#include "llvm/IR/Type.h" //type->dump()

using namespace llvm;

#define DEBUG_TYPE "inject-func-call"

//-----------------------------------------------------------------------------
// InjectFuncCall implementation
//-----------------------------------------------------------------------------
bool InjectFuncCall::runOnModule(Module &M) {
  auto &CTX = M.getContext();

  //define struct type
  auto s_type = StructType::create(CTX, "s_stack");
  //auto s_pointer_type = PointerType::getUnqual(s_type); //TODO: chiedi per address space
  auto s_pointer_type = s_type->getPointerTo();
  ArrayRef<Type*> elements = {IntegerType::getInt32Ty(CTX), s_pointer_type};
  s_type->setBody(elements, true);


  //inject global pointer to the head of the stack
  IRBuilder<> builder(CTX);
  Instruction *I = &*inst_begin(M.getFunction("main"));
  builder.SetInsertPoint(I);
  M.getOrInsertGlobal("stack_head", s_pointer_type);
  GlobalVariable* head_var = M.getNamedGlobal("stack_head");
  head_var->setConstant(false);
  head_var->setLinkage(GlobalValue::InternalLinkage);
  head_var->setInitializer(ConstantPointerNull::get(s_pointer_type));


  //inject call to malloc at the beginning of the main function

  auto malloc_call = CallInst::CreateMalloc(I, 
                                            Type::getInt64Ty(CTX),
                                            s_pointer_type->getPointerElementType(),
                                            builder.getInt64(16),
                                            nullptr,
                                            nullptr,
                                            "malloc_head");
  builder.SetInsertPoint(malloc_call->getNextNode()); //since IRBuilder doesn't have createMalloc
  auto st = builder.CreateStore(malloc_call, head_var, false);
  auto h1 = builder.CreateLoad(head_var, "h1");   //load head
  auto ha1 = builder.CreateGEP(h1, {builder.getInt32(0),builder.getInt32(0)}, "ha"); //get pointer to head->a
  auto sa1 = builder.CreateStore(builder.getInt32(42), ha1); //head->a = 42


  //inject global variable

  M.getOrInsertGlobal("globalKey", builder.getInt64Ty());
  GlobalVariable* g_var = M.getNamedGlobal("globalKey");
  builder.CreateStore(builder.getInt64(11), g_var);
  g_var->setLinkage(GlobalValue::InternalLinkage);
  //g_var->setAlignment(Align(8));
  g_var->setInitializer(builder.getInt64(0));
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
      CTX, "Hello from: %s\n   called at line: %d   global: %d\n");

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
          //before performing the call, push a new line on the stack
          //after performing the call, pop from the stack
          builder.SetInsertPoint(&i);
          auto i_call = dyn_cast<CallInst>(&i);
          auto name_str = i_call->getCalledFunction()->getName();
          auto FuncName = builder.CreateGlobalStringPtr(i_call->getCalledFunction()->getName());
          llvm::Value *FormatStrPtr =
            builder.CreatePointerCast(PrintfFormatStrVar, PrintfArgTy, "formatStr");

          //load global variable
          //auto load = builder.CreateLoad(g_var, "load_g_var");
          //load head->a
          auto h = builder.CreateLoad(head_var, "h");
          auto ha = builder.CreateGEP(h, {builder.getInt32(0), builder.getInt32(0)}, "ha");
          ha = builder.CreateLoad(ha, "haa");
          builder.CreateCall(
            Printf, {FormatStrPtr, FuncName, builder.getInt32(line), ha});
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
