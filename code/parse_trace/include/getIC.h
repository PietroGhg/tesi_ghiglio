#pragma once
#include "boost/algorithm/string.hpp" //string split
#include "llvm/Support/raw_ostream.h"
#include "callGraph.hpp"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DebugInfoMetadata.h" //DISubprogram
#include <functional> //std::function
#include "map.hpp"
#include "sourcelocation.h"
#include "parsejson.h"

using namespace llvm;

class BBTrace{
private:
    int bb_id;
    std::vector<SourceLocation> locations;
public:
    //assumes to receive a string in the shape: bb_id line1 line2 line3
  BBTrace(std::string trace,
	  std::map<unsigned, std::string>& idFileMap){
      std::vector<std::string> splitted;
      boost::algorithm::split(splitted, trace, [](char c){ return c == ' ';});
      bb_id = std::stoi(splitted[0]);
      std::transform(splitted.begin()+1,
		     splitted.end(),
		     std::back_inserter(locations), 
		     [&idFileMap](std::string s){
		       return SourceLocation(std::stoi(s), idFileMap); });
    }
    int getBBid() { return bb_id; }
    const std::vector<SourceLocation>& getLocations() const { return locations; }
    friend raw_ostream& operator<<(raw_ostream& os, const BBTrace& bbt);

};

inline raw_ostream& operator<<(raw_ostream& os, const BBTrace& bbt){
    os << "bb_id: " << bbt.bb_id << ", lines: ";
    for(auto el : bbt.locations){
        os << el << ", ";
    }
    os << "\n";
    return os;
}

using sourcecost_t = std::map<SourceLocation, double>;

inline void addCost(sourcecost_t& sc, SourceLocation& sourceLoc, double cost){

  if(!sc.insert(std::pair<SourceLocation, double>(sourceLoc, cost)).second){
    sc[sourceLoc] = sc[sourceLoc] +  cost;
  }
}
  

inline sourcecost_t getCost(const std::vector<BBTrace>& bbTvec, 
			    const std::vector<BasicBlock*>& bbVec,
			    const CallGraph& cg,
			    std::function<double (Instruction*)>
			    costFunction,
			    bool attributeCallsites){
    //TODO: this should carry also info about which function the line belongs to
  sourcecost_t sc;
  auto main_node = getMainNode(cg);
  auto rec_callsites = getRecursiveCallSites(cg);
  vertex_t curr_node = main_node;
  edge_t in_edge;

  for(auto bbt : bbTvec){
    for(auto& i : *bbVec[bbt.getBBid()]){
      SourceLocation funcLocation(i.getParent()->getParent()->getSubprogram());
      curr_node = main_node;
      auto loc = i.getDebugLoc();
      auto locations = bbt.getLocations();
      auto cost = costFunction(&i);
      if(loc){
	SourceLocation sourceLoc(loc.get());
	addCost(sc, sourceLoc, cost);
      }
      else{
	//the cost of instructions with no location is assigned to
	//their function's definition's location
	addCost(sc, funcLocation, cost);
      }

      
      //assign the cost to the callsites
      if(attributeCallsites) {
	for(auto loc_it = locations.rbegin();
	    loc_it != locations.rend(); loc_it++){
	  if(cg[curr_node].f->getName() == "main"){
	    addCost(sc, *loc_it, cost);
	  }
	  else if(checkIncrease(cg, rec_callsites, *loc_it, in_edge)){
	    addCost(sc, *loc_it, cost);
	  }
	  //move to next node in call graph
	  if(loc_it != locations.rend() - 1){
	    in_edge = getNextEdge(cg, curr_node, *std::next(loc_it));
	    curr_node = target(in_edge, cg);
	  }
	}
      }
    }
  }
  return sc;
}

inline sourcecost_t getIC(const std::vector<BBTrace>& bbTvec, 
			  const std::vector<BasicBlock*>& bbVec,
			  const CallGraph& cg,
			  bool attributeCallsites){
  auto costF = [](Instruction* I){ return 1; };
  return getCost(bbTvec, bbVec, cg, costF, attributeCallsites);
}

inline sourcecost_t getICAss(const std::vector<BBTrace>& bbTvec, 
			     const std::vector<BasicBlock*>& bbVec,
			     const CallGraph& cg,
			     std::map<Instruction*, unsigned long>&
			     instrIndex,
			     LinesInstr& linesAddr,
			     bool attributeCallsites){
  auto cf = [&instrIndex, &linesAddr](Instruction* I){
    return linesAddr[instrIndex[I]].size();
  };
  
  return getCost(bbTvec, bbVec, cg, cf, attributeCallsites);
}

inline sourcecost_t getJoule(const std::vector<BBTrace>& bbTvec,
			     const std::vector<BasicBlock*> bbVec,
			     const CallGraph& cg,
			     std::map<Instruction*, unsigned long>&
			     instrIndex,
			     LinesInstr& linesAddr,
			     costMap_t costMap,
			     bool attributeCallsites) {
  auto cf = [&instrIndex, &linesAddr, &costMap](Instruction* I){
    double cost = 0;
    for(auto& assInstr : linesAddr[instrIndex[I]]) {
      assert(costMap.find(assInstr.getOperation()) != costMap.end() &&
	     "cannot find element in cost map");
      cost += costMap[assInstr.getOperation()];
    }
    return cost;
  };

  return getCost(bbTvec, bbVec, cg, cf, attributeCallsites);
}
			     
