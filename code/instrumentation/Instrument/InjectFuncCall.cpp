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
  
  llvm::Constant *PrintfFormatStr = llvm::ConstantDataArray::getString(
      CTX, "Executing basic block: %d\tfrom line: %d\n");
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

FunctionCallee createPush(LLVMContext& CTX, Module& M){
  IRBuilder<> builder(CTX);
  GlobalVariable* head_var = M.getNamedGlobal("stack_head");
  FunctionType* push_ty = FunctionType::get(Type::getVoidTy(CTX),
                                           Type::getInt32Ty(CTX),
                                           false);
  FunctionCallee push = M.getOrInsertFunction("push", push_ty);
  Function* pushF = dyn_cast<Function>(push.getCallee());
  BasicBlock* bb = BasicBlock::Create(CTX, "push_bb", pushF);
  builder.SetInsertPoint(bb);
  //allocate space for temp on the stack
  auto alloca_temp = builder.CreateAlloca(head_var->getType()->getElementType(), nullptr, "alloca_temp");
  //load head
  auto load_head = builder.CreateLoad(head_var, "load_head");
  //allocate memory for the new stack element
  auto head_type = dyn_cast<PointerType>(head_var->getType()->getElementType());
  auto alloc_type = head_type->getElementType();

  auto malloc_temp = CallInst::CreateMalloc(load_head, 
                                            Type::getInt64Ty(CTX),
                                            head_type->getPointerElementType(),
                                            ConstantExpr::getSizeOf(alloc_type),
                                            nullptr,
                                            nullptr,
                                            "malloc_temp");
  
  builder.SetInsertPoint(bb);
  builder.CreateStore(malloc_temp, alloca_temp, false);
  //temp->a = line
  auto load_temp = builder.CreateLoad(alloca_temp, "load_temp");
  auto tempa = builder.CreateGEP(load_temp, {builder.getInt32(0), builder.getInt32(0)}, "temp-a");
  builder.CreateStore(pushF->getArg(0), tempa);
  //temp->prev = head
  auto temp_prev = builder.CreateGEP(load_temp, {builder.getInt32(0), builder.getInt32(1)}, "temp-prev");
  builder.CreateStore(load_head, temp_prev, false);
  //head = temp
  builder.CreateStore(load_temp, head_var, false);
  builder.CreateRet(nullptr);

  return std::move(push);
}

FunctionCallee createPop(LLVMContext& CTX, Module& M){
  IRBuilder<> builder(CTX);
  GlobalVariable* head_var = M.getNamedGlobal("stack_head");
  FunctionType* pop_ty = FunctionType::get(Type::getVoidTy(CTX),
                                          false);
  FunctionCallee pop = M.getOrInsertFunction("pop", pop_ty);
  Function* popF = dyn_cast<Function>(pop.getCallee());
  BasicBlock* bb = BasicBlock::Create(CTX, "pop_bb", popF);
  builder.SetInsertPoint(bb);

  auto alloca_temp = builder.CreateAlloca(head_var->getType()->getPointerElementType(), nullptr, "alloca_temp");
  //temp = head
  auto load_head = builder.CreateLoad(head_var, "load-head");
  builder.CreateStore(load_head, alloca_temp);
  //head = head -> prev
  auto h_prev_ptr = builder.CreateGEP(load_head, {builder.getInt32(0), builder.getInt32(1)}, "h_prev_ptr");
  auto h_prev = builder.CreateLoad(h_prev_ptr, "h_prev");
  builder.CreateStore(h_prev, head_var);
  //free(temp)
  auto load_temp = builder.CreateLoad(alloca_temp, "load_temp");
  auto ret = builder.CreateRet(nullptr);
  CallInst::CreateFree(load_temp, ret);
  

  return std::move(pop);
}

bool isInjected(Function* f){
  auto name = f->getName();
  return name == "pop" || name == "push" || name == "print_stack";
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
                                            ConstantExpr::getSizeOf(s_type),
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

  
  FunctionCallee print_stack = createPrintStack(CTX, M);
  FunctionCallee push = createPush(CTX, M);
  FunctionCallee pop = createPop(CTX, M);


  for (auto &F : M) {
    if (F.isDeclaration() || isInjected(&F))
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
          //check if it is a call to an injected function
          CallInst* c_inst = dyn_cast<CallInst>(&i);
          if(isInjected(c_inst->getCalledFunction()))
            continue;
          //else
          auto line = i.getDebugLoc().getLine();
          //before performing the call, push a new line on the stack
          CallInst::Create(push, {builder.getInt32(line)}, None, "", &i);
          //after performing the call, pop from the stack
          CallInst::Create(pop, "", i.getNextNode());

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
