#pragma once
#include <string>
#include "llvm/IR/DebugInfoMetadata.h"

class SourceLocation {
 private:
  unsigned Line;
  unsigned Column;
  std::string File;
 public:
  SourceLocation(unsigned Line, unsigned Column, const std::string& File);
  SourceLocation(llvm::DILocation* Loc);
  bool operator==(const SourceLocation& Other) const;
  bool operator<(const SourceLocation& Other) const;
  unsigned getLine() const { return Line; }
  unsigned getColumn() const { return Column; }
  const std::string& getFile() const { return File; }
  std::string toString() const ;
};
