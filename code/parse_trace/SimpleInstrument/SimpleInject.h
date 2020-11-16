#pragma once

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
struct SimpleInject : public llvm::PassInfoMixin<SimpleInject> {
  llvm::PreservedAnalyses run(llvm::Module &M,
                              llvm::ModuleAnalysisManager &);
  bool runOnModule(llvm::Module &M);
};

//------------------------------------------------------------------------------
// Legacy PM interface
//------------------------------------------------------------------------------
struct LegacySimpleInject : public llvm::ModulePass {
  static char ID;
  LegacySimpleInject() : ModulePass(ID) {}
  bool runOnModule(llvm::Module &M) override;

  SimpleInject Impl;
};

