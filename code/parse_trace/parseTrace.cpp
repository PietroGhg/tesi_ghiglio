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
#include <fstream>
#include <set>
#include "FilesUtils.h"
#include "sourcelocation.h"
#include "callGraph.hpp"
#include "map.hpp"
#include "getIC.h"
#include "parsejson.h"

using namespace llvm;



//! Function that opens a trace file and returns a vector of the bbtraces contained in it
std::vector<BBTrace> getBBTraceVec(const std::string& path,
				   std::map<unsigned, std::string>& idFileMap){
    std::vector<BBTrace> result;
    std::ifstream f(path);
    std::string line;
    if(!f.is_open()){
        errs() << "error while opening trace file\n";
        throw;
    }

    while(std::getline(f, line)){
      result.push_back(BBTrace(line, idFileMap));
    }
    f.close();
    return result;  
}

//! Function that returns a vector of all the basic blocks in the module
std::vector<BasicBlock*> getBBvec(Module* m){
    std::vector<BasicBlock*> result;
    for(auto& f : *m){
        for(auto& bb : f){
            result.push_back(&bb);
        }
    }
    return result;
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

void printAnnotatedFile(const string& sourcePath,
			sourcecost_t& sc,
			const string& metric){
  ifstream source(sourcePath);
  assert(source.is_open() && "Unable to open source file"); 

  int numLines = getNumLines(source);

  //vector where each line contains the cost associated to the line
  std::vector<double> linesCosts(numLines+1);
  for(auto& el : linesCosts)
    el = 0;

  //update the vector of costs
  for(auto& el : sc){
    if(el.first.getFile() == sourcePath)
      linesCosts[el.first.getLine()] += el.second;
  }

  //prints the source code and the costs
  std::string line;
  int i = 1;
  errs() << "\nFile: " << sourcePath << "\n";
  while(getline(source, line)){
    if(linesCosts[i] != 0){
      errs() << i << ": " << line << " //" << linesCosts[i] << " " << metric << "\n";
    }
    else{
      errs() << i << ": " << line << "\n";
    }
    i++;
  }
  //errs() << "total: " << linesCosts[0] << "\n";//TODO: fix this
  source.close();
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
    
    
    auto idFileMap = getIDFileMap(*m);
    auto bb_trace_vec = getBBTraceVec(argv[2], idFileMap);
    auto bb_vec = getBBvec(m.get());

    auto files = getFiles(*m);
    auto scIR = getIC(bb_trace_vec, bb_vec, cg);

    for(auto& f : files){
      printAnnotatedFile(f, scIR, "llvm instr");
    }

    //get the source location -> assembly map
    auto theMap = getMap(argv[3], *m);

    //get the instruction -> id map
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
    for(auto& el : reversed)
      errs() << *el.second << " " << el.first << "\n";

    //get the map using assembly ic as metric
    auto scAss = getICAss(bb_trace_vec, bb_vec, cg, instrMap, theMap);

    //print the annotated files
    for(auto& f : files){
      printAnnotatedFile(f, scAss, "assembly inst");
    }
}


