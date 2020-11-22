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
#include "llvm/Support/CommandLine.h"
#include "boost/algorithm/string.hpp" //string split
#include "algorithm" //std::transform
#include "fstream" //ifstream
#include <fstream>
#include <set>
#include "FilesUtils.h"
#include "sourcelocation.h"
#include "callGraph.h"
#include "map.h"
#include "getIC.h"
#include "costmap.h"
#include "extendCall.h"

using namespace llvm;

//command line options
static cl::opt<bool>
llvmInstr("llvm", cl::desc("Use number of llvm instructions as metric"),
	  cl::init(false));

static cl::opt<bool>
assInstr("ass", cl::desc("Use number of assembly instructions as metric"),
	 cl::init(false));

static cl::opt<bool>
energy("energy", cl::desc("Use energy as metric"), cl::init(false));

static cl::opt<bool>
callsites("callsites", cl::desc("Attribute cost to callsites"), cl::init(false));

static cl::opt<bool>
simple("simple", cl::desc("True if trace has been produced with the simple intrumentation and requires expansion"), cl::init(true));

static cl::opt<bool>
testExp("testExp", cl::desc("Test if the expansion went alright"), cl::init(false));

static cl::opt<bool>
printCallGr("printCG", cl::desc("Print call graph"), cl::init(false));

static cl::opt<bool>
printDisAss("printDisAss", cl::desc("Print disassembly"), cl::init(false));

static cl::opt<std::string>
module("m", cl::desc("Input module"));

static cl::opt<std::string>
binary("b", cl::desc("Binary file for disassembly"));

static cl::opt<std::string>
trace("t", cl::desc("Trace file"));

static cl::opt<std::string>
json("json", cl::desc("json cpu description"));



//! Function that opens a trace file and returns a vector of the bbtraces contained in it
std::vector<BBTrace> getBBTraceVec(const std::string& path,
				   std::map<unsigned, std::string>& idFileMap){
    std::vector<BBTrace> result;
    std::ifstream f(path);
    std::string line;
    assert(f.is_open() && "error while opening trace file\n");


    while(std::getline(f, line)){
      result.emplace_back(line, idFileMap);
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

void printInstrMap(std::map<Instruction*, unsigned long> instrMap){
  std::map<unsigned long, Instruction*> reversed;
  for(auto& el : instrMap)
    reversed[el.second] = el.first;
  for(auto& el : reversed){
    errs() << el.first << ": ";
    auto& I = *el.second;
    errs() << I << " ";
    if(auto loc = I.getDebugLoc()){
      auto file = I.getParent()->getParent()->getSubprogram()->getFilename();
      errs() << loc.getLine() << " " << file;
    }
    errs() << "\n";
  }
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
  source.close();
}

std::map<Instruction*, unsigned long> getInstrMap(Module& m){
    //get the instruction -> id map
    std::map<Instruction*, unsigned long> instrMap;
    unsigned long index = 1;
    for(auto& f : m){
      for(auto& bb : f){
	for(auto& i : bb){
	  instrMap.insert(std::pair<Instruction*, unsigned long>(&i, index));
	  index++;
	}
      }
    }
    return instrMap;
}
    

int main(int argc, char* argv[]){

  //parse command line
  cl::ParseCommandLineOptions(argc, argv);
  if(module.empty()){
    errs() << "no module specified, exiting\n";
    return -1;
  }

  if(assInstr && binary.empty()){
    errs() << "assembly instruction requested but no binary provided\n";
    return -1;
  }

  if(energy && json.empty()){
    errs() << "energy requested but no json provided\n";
    return -1;
  }
  
  if(trace.empty()){
    errs() << "no trace specified, exiting\n";
    return -1;
  }
  LLVMContext c;

  SMDiagnostic err;
  auto m = parseIRFile(module, err, c);
  assert(m && "error while opening .ll file\n");
  auto cg = makeCallGraph(m.get());
  if(printCallGr)
    errs() << getDOT(cg) << "\n";    

  auto bb_vec = getBBvec(m.get());
  vector<BBTrace> bb_trace_vec;

  if(simple){
    auto simpleBBV = getSimpleBBTVec(trace);
    auto extBBVec = getExtBBVec(bb_vec, simpleBBV);
    bb_trace_vec = extendBBT(extBBVec);
    
  }
  else{
      auto idFileMap = getIDFileMap(*m);
      auto oldVec = getBBTraceVec(trace, idFileMap);

      auto simpleBBV = getSimpleBBTVec(trace);
      auto extBBVec = getExtBBVec(bb_vec, simpleBBV);
      bb_trace_vec = extendBBT(extBBVec);

      //test the extended version
      if(testExp){
	assert(extBBVec.size() == bb_trace_vec.size());
	assert(bb_trace_vec.size() == oldVec.size());
	for(int i = 0; i < bb_trace_vec.size(); i++){
	  assert(bb_trace_vec[i].getBBid() == oldVec[i].getBBid());
	  assert(bb_trace_vec[i].getLocations().size() == oldVec[i].getLocations().size()-1);
	  
	  for(int j = 0; j < bb_trace_vec[i].getLocations().size(); j++){
	    assert(bb_trace_vec[i].getLocations()[j] == oldVec[i].getLocations()[j]);
	    
	  }
	}
      }
      
  }
    

  auto files = getFiles(*m);

  if(llvmInstr) {
    auto scIR = getIC(bb_trace_vec, bb_vec, cg, callsites);
    for(auto& f : files){
      printAnnotatedFile(f, scIR, "llvm instr");
    }
  }


  LinesInstr theMap;
  std::map<Instruction*, unsigned long> instrMap; //instr->id  map
  if(assInstr || energy || printDisAss) {
    //get the source location -> assembly map
    theMap = getMap(binary, *m, printDisAss);
    instrMap = getInstrMap(*m);
    if(printDisAss){
      printInstrMap(instrMap);
    }
  }
    

  if(assInstr) {
    //get the map using assembly ic as metric
    auto scAss = getICAss(bb_trace_vec, bb_vec, cg, instrMap, theMap, callsites);

    //print the annotated files
    for(auto& f : files){
      printAnnotatedFile(f, scAss, "assembly inst");
    }
  }

  //joule as metric
  //180mhz, 5907 microW
  if(energy) {
    auto costMap = getCostMap(json);
    auto scJoule = getJoule(bb_trace_vec, bb_vec, cg ,instrMap, theMap,
			    costMap, callsites);
    for(auto& f : files){
      printAnnotatedFile(f, scJoule, "microJoule");
    }
  }
}


