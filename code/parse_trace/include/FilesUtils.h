#pragma once
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

inline map<unsigned, string> getIDFileMap(const Module& M){
  auto files = getFiles(M);
  map<unsigned, string> res;
  unsigned i = 0;
  for(auto& f : files){
    res[i] = f;
    i++;
  }
  return res;
}

inline map<string, unsigned> getFileIDMap(const Module& M){
  auto files = getFiles(M);
  map<string, unsigned> res;
  unsigned i = 0;
  for(auto& f : files){
    res[f] = i;
    i++;
  }
  return res;
}
