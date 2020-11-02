#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h" //path exists
#include "llvm/Support/Path.h"
#include <set>
using namespace std;
using namespace llvm;

//taken from GCOVProfiling.cpp
inline std::string getFilename(const DISubprogram *SP) {
  SmallString<128> Path;
  StringRef RelPath = SP->getFilename();
  if (sys::fs::exists(RelPath))
    Path = RelPath;
  else
    sys::path::append(Path, SP->getDirectory(), SP->getFilename());
  return Path.str().str();
}

inline std::set<std::string> getFiles(const Module& M){
  std::set<std::string> res;
  for(auto& F : M){
    if(auto sp = F.getSubprogram())
      res.insert(getFilename(sp));
  }
  return res;
}
