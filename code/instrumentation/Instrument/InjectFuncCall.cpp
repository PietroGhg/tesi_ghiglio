#include "InjectFuncCall.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

#include "llvm/IR/GlobalValue.h" //global variable linkage types

using namespace llvm;

#define DEBUG_TYPE "inject-func-call"

FunctionCallee createPrintStack(LLVMContext& CTX, Module& M){
  IRBuilder<> builder(CTX);

  GlobalVariable* head_var = M.getNamedGlobal("stack_head");
  if(!head_var) errs()<< "ok\n";
  
  llvm::Constant *PrintfFormatStr = llvm::ConstantDataArray::getString(
      CTX, "Hello from basic block: %d\n\texecuted from line: %d\n");
  Constant *PrintfFormatStrVar =
      M.getOrInsertGlobal("PrintfFormatStr", PrintfFormatStr->getType());
  dyn_cast<GlobalVariable>(PrintfFormatStrVar)->setInitializer(PrintfFormatStr);

  Function* Printf = M.getFunction("printf");
  auto PrintfArgTy = Printf->getFunctionType()->params()[0];
  

  FunctionType* print_stack_ty = FunctionType::get(Type::getVoidTy(CTX),
                                                  {Type::getInt32Ty(CTX)},
                                                  false);
  FunctionCallee print_stack = M.getOrInsertFunction("print_stack", print_stack_ty);
  Function* print_stackF = dyn_cast<Function>(print_stack.getCallee());
  //TODO: attributes?
  BasicBlock* bb1 = BasicBlock::Create(CTX, "print_stack_bb1", print_stackF);
  builder.SetInsertPoint(bb1);
  auto i = builder.CreateLoad(head_var, "load_head");
  auto i2 = builder.CreateGEP(i, {builder.getInt32(0), builder.getInt32(0)}, "ha_ref");
  auto i3 = builder.CreateLoad(i2, "load_ha");
  llvm::Value *FormatStrPtr =
      builder.CreatePointerCast(PrintfFormatStrVar, PrintfArgTy, "formatStr");
  builder.CreateCall(
      Printf, {FormatStrPtr, print_stackF->getArg(0), i3});
  builder.CreateRet(nullptr);

  return std::move(print_stack);
}

//-----------------------------------------------------------------------------
// InjectFuncCall implementation
//-----------------------------------------------------------------------------
bool InjectFuncCall::runOnModule(Module &M) {
  auto &CTX = M.getContext();

  //define struct type
  auto s_type = StructType::create(CTX, "s_stack");
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
  auto sa1 = builder.CreateStore(builder.getInt32(42), ha1); //TODO: set actual line of main


  bool injectedAtLeastOnce = false;
  unsigned bb_count = 0;

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

  //definition of print_stack: void print_stack(int bb_id)
  FunctionCallee print_stack = createPrintStack(CTX, M);


  for (auto &F : M) {
    if (F.isDeclaration() || F.getName() == "print_stack")
      continue;

    for(auto& bb : F){
        //instrument basic block in order to print stack at the beginning of its execution
        if(F.getName() != "main")
          builder.SetInsertPoint(&*(bb.getFirstInsertionPt()));
        else
          builder.SetInsertPoint(sa1->getNextNode());
        //inject a call to print_stack(bb_count)
        builder.CreateCall(print_stack, {builder.getInt32(bb_count)});
        bb_count++;
        
      for(auto& i : bb){
        if(isa<CallInst>(i) && i.getDebugLoc() && !isa<DbgInfoIntrinsic>(i)){
          //before performing the call, push a new line on the stack
          //after performing the call, pop from the stack

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
