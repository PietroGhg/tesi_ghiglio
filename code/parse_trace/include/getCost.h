#pragma once
#include "boost/algorithm/string.hpp" //string split
#include "llvm/IR/Instruction.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DebugInfoMetadata.h" //DISubprogram
#include <functional> //std::function
#include <numeric> //std::accumulate
#include "map.h"
#include "callGraph.h"
#include "sourcelocation.h"
#include "costmap.h"


using namespace llvm;

class BBTrace{
private:
    int bb_id;
    std::vector<SourceLocation> locations;
public:
  //assumes to receive a string in the shape: bb_id line1 line2 line3
  BBTrace(std::string trace,
	  std::map<unsigned, std::string>& idFileMap);

  //sets only the id
  BBTrace(std::string trace);
  
  BBTrace(int bb_id, std::vector<SourceLocation> locations);
  
  int getBBid() const { return bb_id; }
  const std::vector<SourceLocation>& getLocations() const { return locations; }
  friend raw_ostream& operator<<(raw_ostream& os, const BBTrace& bbt);

};


using sourcecost_t = std::map<SourceLocation, double>;


//! Computes only the total cost, no source attribution
double getTotalCost(const std::vector<BBTrace>& bbTvec,
		    const std::vector<BasicBlock*>& bbVec,
		    std::function<double (Instruction*)> cf);
  

//! Computes cost with source code attribution
sourcecost_t getCost(const std::vector<BBTrace>& bbTvec, 
		     const std::vector<BasicBlock*>& bbVec,
		     const CallGraph& cg,
		     std::function<double (Instruction*)>
		     costFunction,
		     bool attributeCallsites);

//! Computes LLVM instruction count
sourcecost_t getIC(const std::vector<BBTrace>& bbTvec, 
		   const std::vector<BasicBlock*>& bbVec,
		   const CallGraph& cg,
		   bool attributeCallsites);

//! Total LLVM ic
double getTotalLLVM(const std::vector<BBTrace>& bbTvec,
		    const std::vector<BasicBlock*>& bbVec);

//! Assembly ic
sourcecost_t getICAss(const std::vector<BBTrace>& bbTvec, 
		      const std::vector<BasicBlock*>& bbVec,
		      const CallGraph& cg,
		      std::map<Instruction*, unsigned long>&
		      instrIndex,
		      LinesInstr& linesAddr,
		      bool attributeCallsites);

//! Total assembly ic
double getTotalAss(const std::vector<BBTrace>& bbTvec, 
		   const std::vector<BasicBlock*>& bbVec,
		   std::map<Instruction*, unsigned long>&
		   instrIndex,
		   LinesInstr& linesAddr);

//! Energy consumption
sourcecost_t getJoule(const std::vector<BBTrace>& bbTvec,
		      const std::vector<BasicBlock*> bbVec,
		      const CallGraph& cg,
		      std::map<Instruction*, unsigned long>&
		      instrIndex,
		      LinesInstr& linesAddr,
		      costMap_t costMap,
		      bool attributeCallsites);

//! Total energy consumption
double getTotalJoule(const std::vector<BBTrace>& bbTvec,
		     const std::vector<BasicBlock*> bbVec,
		     std::map<Instruction*, unsigned long>&
		     instrIndex,
		     LinesInstr& linesAddr,
		     costMap_t costMap);			     


//! Source attribution given basic blocks count
sourcecost_t getCost(std::vector<BasicBlock*>& bbVec,
		     std::vector<uint64_t>& counts,
		     std::function<double (Instruction*)> costFunction);

//! Total cost given basic blocks count
double getTotalCost(std::vector<BasicBlock*>& bbVec,
		    std::vector<uint64_t>& counts,
		    std::function<double (Instruction*)> costFunction);

//! Counts how many times each assembly instruction has been executed
map<string, unsigned long>
getInstrCount(const std::vector<BasicBlock*> bbVec,
	      vector<uint64_t> counts,
	      std::map<Instruction*, unsigned long>&
	      instrIndex,
	      LinesInstr& linesAddr);
