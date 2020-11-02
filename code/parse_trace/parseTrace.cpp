#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h" //SMDiagnostic
#include "llvm/IR/DebugInfoMetadata.h" //DISubprogram
#include "boost/algorithm/string.hpp" //string split
#include "algorithm" //std::transform
#include "fstream" //ifstream
#include <set>
#include "FilesUtils.h"
#include "callGraph.hpp"
#include "map.hpp"
#include "getIC.h"
using namespace llvm;


//! Function that opens a trace file and returns a vector of the bbtraces contained in it
std::vector<BBTrace> getBBTraceVec(std::string path){
    std::vector<BBTrace> result;
    std::ifstream f(path);
    std::string line;
    if(!f.is_open()){
        errs() << "error while opening trace file\n";
        throw;
    }

    while(std::getline(f, line)){
        result.push_back(BBTrace(line));
    }
    f.close();
    return std::move(result);  
}

//! Function that returns a vector of all the basic blocks in the module
std::vector<BasicBlock*> getBBvec(Module* m){
    std::vector<BasicBlock*> result;
    for(auto& f : *m){
        for(auto& bb : f){
            result.push_back(&bb);
        }
    }
    return std::move(result);
}

int getNumLines(std::ifstream& source){
    std::string line;
    int ris = 0;
    while(std::getline(source, line)){
        ris++;
    }
    source.clear();                
    source.seekg(0, std::ios::beg);
    return ris;
}
    
  

//1 -> original module .ll
//2 -> trace
//3 -> executable with replaced debug info
int main(int argc, char* argv[]){
    LLVMContext c;

    SMDiagnostic err;
    auto m = parseIRFile(argv[1], err, c);
    if(!m){
        errs() << "error while opening .ll file\n";
        return -1;
    }
    auto cg = makeCallGraph(m.get());
    errs() << getDOT(cg) << "\n";    
    
    
    
    auto bb_trace_vec = getBBTraceVec(argv[2]);
    auto bb_vec = getBBvec(m.get());
    auto theMap = getMap(argv[3], *m);
    auto files = getFiles(*m);
    for(auto& f : files){
      errs() << "Analizing " << f << "\n";
      std::ifstream source(f);
      if(!source.is_open()){
	errs() << "error while opening source file.\n";
	return -1;
      }
      int num_lines = getNumLines(source);
      auto ic = getIC(num_lines, bb_trace_vec, bb_vec, cg);

      std::string line;
      int i = 1;
      while(getline(source, line)){
	if(ic[i] != 0){
	  errs() << i << ": " << line << " //" << ic[i] << " llvm instr\n";
	}
	else{
	  errs() << i << ": " << line << "\n";
	}
	i++;
      }
      errs() << "total: " << ic[0] << "\n";

      //theMap

      std::map<Instruction*, unsigned long> instrMap;
      unsigned long index = 1;
      for(auto& f : *m){
	for(auto& bb : f){
	  for(auto& i : bb){
	    instrMap[&i] = index;
	    index++;
	  }
	}
      }
    
      std::map<unsigned long, Instruction*> reversed;
      for(auto& el : instrMap){
	reversed[el.second] = el.first;
      }
      /*for(auto& el : reversed){
	errs() << el.first << ": " << *el.second << "\n";
      }*/
   


      auto icAss = getICAss(num_lines, bb_trace_vec, bb_vec, cg, instrMap, theMap);
      source.clear(); 
      source.seekg(0, std::ios::beg);
      i = 1;
      while(getline(source, line)){
	if(icAss[i] != 0){
	  errs() << i << ": " << line << " //" << icAss[i] << " assembly instr\n";
	}
	else{
	  errs() << i << ": " << line << "\n";
	}
	i++;
      }
      errs() << "total: " << icAss[0] << "\n";
    }
}
