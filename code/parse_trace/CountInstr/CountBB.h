#pragma once

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
struct CountBB : public llvm::PassInfoMixin<CountBB> {
  llvm::PreservedAnalyses run(llvm::Module &M,
                              llvm::ModuleAnalysisManager &);
  bool runOnModule(llvm::Module &M);
};

//------------------------------------------------------------------------------
// Legacy PM interface
//------------------------------------------------------------------------------
struct LegacyCountBB : public llvm::ModulePass {
  static char ID;
  LegacyCountBB() : ModulePass(ID) {}
  bool runOnModule(llvm::Module &M) override;

  CountBB Impl;
};

