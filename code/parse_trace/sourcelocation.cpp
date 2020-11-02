#include "sourcelocation.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"

SourceLocation::SourceLocation(unsigned Line,
			       unsigned Column,
			       const std::string& File):
  Line(Line), Column(Column), File(File){}

//taken from GCOVProfiling.cpp
static std::string getFilename(const llvm::DISubprogram *SP) {
  llvm::SmallString<128> Path;
  llvm::StringRef RelPath = SP->getFilename();
  if (llvm::sys::fs::exists(RelPath))
    Path = RelPath;
  else
    llvm::sys::path::append(Path, SP->getDirectory(), SP->getFilename());
  return Path.str().str();
}

SourceLocation::SourceLocation(llvm::DILocation* Loc){
  assert(Loc && "Location must not be null");
  llvm::DILocalScope *Scope = Loc->getScope();
  File = getFilename(Scope->getSubprogram());  
  Line = Loc->getLine();
  Column = Loc->getColumn();
}

bool SourceLocation::operator==(const SourceLocation& Other) const {
  return Line == Other.getLine() &&
    Column == Other.getColumn() &&
    File == Other.getFile();
}

bool SourceLocation::operator<(const SourceLocation& Other) const {
  if(File < Other.getFile())
    return true;
  if(Line < Other.getLine())
    return true;
  if(Column < Other.getColumn())
    return true;
  return false;
}

std::string SourceLocation::toString() const {
  return "Line: " + std::to_string(Line) + " Col: " +
    std::to_string(Column) + " File: " + File;
}
