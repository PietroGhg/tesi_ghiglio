#include "sourcelocation.h"
#include "include/FilesUtils.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

using namespace std;

FunctionLocation::FunctionLocation(llvm::DISubprogram* SubPr):
  Line(SubPr->getLine()), Column(0), File(getFilename(SubPr)), Name(SubPr->getName().str()){}

bool FunctionLocation::operator<(const FunctionLocation& Other) const {
  if(File < Other.getFile())
    return true;
  else if(File > Other.getFile())
    return false;
  
  if(Line < Other.getLine())
    return true;
  else if(Line > Other.getLine())
    return false;

  if(Column < Other.getColumn())
    return true;
  else if (Column > Other.getColumn())
    return false;
  
  return false;
}

raw_ostream& operator<<(raw_ostream& os,
			const FunctionLocation& fl){
  os << fl.Name << " " << fl.Line << " " << fl.Column << " " << fl.File;
  return os;
}

SourceLocation::SourceLocation(unsigned Line,
			       unsigned Column,
			       const std::string& File):
  Line(Line), Column(Column), File(File){}


SourceLocation::SourceLocation(llvm::DILocation* Loc):
  FunLoc(Loc->getScope()->getSubprogram()){
  assert(Loc && "Location must not be null");
  llvm::DILocalScope *Scope = Loc->getScope();
  File = getFilename(Scope->getSubprogram());  
  Line = Loc->getLine();
  Column = Loc->getColumn();
}

SourceLocation::SourceLocation(llvm::DISubprogram* SubPr):
  FunLoc(SubPr){
  assert(SubPr && "Subprogram must not be null");
  File = getFilename(SubPr);
  Line = SubPr->getLine();
  Column = 0;
}

SourceLocation::SourceLocation(unsigned long id,
			       map<unsigned, string>& idFileMap){
  Tri tri(id);
  Line = tri.getFirst();
  Column = tri.getSecond();
  File = idFileMap[tri.getThird()];
}
			       

bool SourceLocation::operator==(const SourceLocation& Other) const {
  return Line == Other.getLine() &&
    Column == Other.getColumn() &&
    File == Other.getFile();
}

bool SourceLocation::operator<(const SourceLocation& Other) const {
  if(File < Other.getFile())
    return true;
  else if(File > Other.getFile())
    return false;
  
  if(Line < Other.getLine())
    return true;
  else if(Line > Other.getLine())
    return false;

  if(Column < Other.getColumn())
    return true;
  else if (Column > Other.getColumn())
    return false;
  
  return false;
}

unsigned long SourceLocation::getSingle(map<string, unsigned>& fileIDMap){
  Tri tri(Line, Column, fileIDMap[File]);
  return tri.getSingle();
}

std::string SourceLocation::toString() const {
  return "Line: " + std::to_string(Line) + " Col: " +
    std::to_string(Column) + " File: " + File;
}

raw_ostream& operator<<(raw_ostream& os,
			       const SourceLocation& sl){
  os << sl.toString();
  return os;
}
