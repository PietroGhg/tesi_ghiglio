// opt -load=./CountInstr/CountBB.so -legacy-count-bb -S test3.l
#include "CountBB.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h" //PHI
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;

PreservedAnalyses CountBB::run(llvm::Module &M,
                                       llvm::ModuleAnalysisManager &) {
  bool Changed =  runOnModule(M);

  return (Changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

bool LegacyCountBB::runOnModule(llvm::Module &M) {
  bool Changed = Impl.runOnModule(M);

  return Changed;
}

uint64_t countBBs(const Module& M){
  uint64_t res = 0;
  for(auto& F : M)
    for(auto& BB :F)
      res++;
  
  return res;
}

FunctionCallee getPrintBBCount(Module& M, uint64_t bbCount){ 
  auto& Ctx = M.getContext();

  //we need printf
  PointerType *PrintfArgTy = PointerType::getUnqual(Type::getInt8Ty(Ctx));


  FunctionType *PrintfTy = FunctionType::get(
      IntegerType::getInt32Ty(Ctx),
      PrintfArgTy,
      true); //isVarArgs


  FunctionCallee Printf = M.getOrInsertFunction("printf", PrintfTy);

  Function *PrintfF = dyn_cast<Function>(Printf.getCallee());
  PrintfF->setDoesNotThrow();
  PrintfF->addParamAttr(0, Attribute::NoCapture);
  PrintfF->addParamAttr(0, Attribute::ReadOnly);

  //format string for the basick block count
  llvm::Constant *bb_id_str = llvm::ConstantDataArray::getString(
      Ctx, "%d\n");
  Constant *bb_id_str_var =
      M.getOrInsertGlobal("bb_id_str", bb_id_str->getType());
  dyn_cast<GlobalVariable>(bb_id_str_var)->setInitializer(bb_id_str);
  
  IRBuilder<> builder(Ctx);
  
  GlobalVariable* vec = M.getNamedGlobal("bbCount");

  //define function
  FunctionType* ty = FunctionType::get(builder.getVoidTy(), false);
  FunctionCallee printBBCount = M.getOrInsertFunction("printBBCount", ty);
  Function* pbbc = dyn_cast<Function>(printBBCount.getCallee());

  //add basic blocks
  BasicBlock* bb_entry = BasicBlock::Create(Ctx, "entry", pbbc);
  BasicBlock* bb_loop = BasicBlock::Create(Ctx, "loop", pbbc);
  BasicBlock* bb_exit = BasicBlock::Create(Ctx, "exit", pbbc);

  //from entry, branch straight to loop
  builder.SetInsertPoint(bb_entry);
  builder.CreateBr(bb_loop);

  //loop
  builder.SetInsertPoint(bb_loop);
  PHINode* phi = builder.CreatePHI(builder.getInt64Ty(), 2, "phi");
  auto bbca = builder.CreateGEP(vec, {builder.getInt64(0), phi}, "bbca");
  auto val = builder.CreateLoad(bbca, "val");
  auto printf_ty = PrintfF->getFunctionType();
  auto printf_arg_ty = printf_ty->params()[0];
  llvm::Value* bb_id_ptr =
    builder.CreatePointerCast(bb_id_str_var, printf_arg_ty, "formatStr");
  auto c = builder.CreateCall(Printf, {bb_id_ptr, val});
  auto inc = builder.CreateAdd(phi, builder.getInt64(1), "inc");
  auto cmp = builder.CreateCmp(CmpInst::Predicate::ICMP_EQ,
			       inc,
			       builder.getInt64(bbCount), "cmp");
  builder.CreateCondBr(cmp, bb_exit, bb_loop);
  //add incoming to phi
  phi->addIncoming(inc, bb_loop);
  phi->addIncoming(builder.getInt64(0), bb_entry);

  //exit
  builder.SetInsertPoint(bb_exit);
  builder.CreateRetVoid();

  return printBBCount;
}

bool CountBB::runOnModule(Module& M){
  auto& CTX = M.getContext();
  uint64_t bbCount = 0;
  IRBuilder<> Builder(CTX);

  //inject array with size = # basic blocks
  unsigned long bbNum = countBBs(M);
  auto arrayTy = ArrayType::get(Builder.getInt64Ty(), bbNum);
  M.getOrInsertGlobal("bbCount", arrayTy);
  GlobalVariable* bbCountVar = M.getNamedGlobal("bbCount");
  ConstantAggregateZero* constInit = ConstantAggregateZero::get(arrayTy);
  bbCountVar->setInitializer(constInit);

  //print counts when exiting
  auto printBBC = getPrintBBCount(M, bbNum);
  appendToGlobalDtors(M, dyn_cast<Function>(printBBC.getCallee()),0);
					      
  for(auto& F : M){
    for(auto& BB : F){
      //increase by one the count of this bb
      Builder.SetInsertPoint(&*(BB.getFirstInsertionPt()));
      auto bbca = Builder.CreateConstInBoundsGEP2_64(arrayTy, bbCountVar, 0, bbCount, "bbca");
      auto oldC = Builder.CreateLoad(bbca, "oldC");
      auto add1 = Builder.CreateAdd(oldC, Builder.getInt64(1));
      Builder.CreateStore(add1, bbca);
      
      bbCount++;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getCountBBPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "count-bb", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "count-bb") {
                    MPM.addPass(CountBB());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getCountBBPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyCountBB::ID = 0;

// Register the pass - required for (among others) opt
static RegisterPass<LegacyCountBB>
    X(/*PassArg=*/"legacy-count-bb", /*Name=*/"LegacyCountBB",
      /*CFGOnly=*/false, /*is_analysis=*/false);
