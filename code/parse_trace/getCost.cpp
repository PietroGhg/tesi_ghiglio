#include "include/map.h"
#include<getCost.h>


BBTrace::BBTrace(std::string trace,
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

BBTrace::BBTrace(std::string trace){
  std::vector<std::string> splitted;
  boost::algorithm::split(splitted, trace, [](char c){ return c == ' ';});
  bb_id = std::stoi(splitted[0]);
}

BBTrace::BBTrace(int bb_id, std::vector<SourceLocation> locations):
  bb_id(bb_id), locations(locations){}

raw_ostream& operator<<(raw_ostream& os, const BBTrace& bbt){
    os << "bb_id: " << bbt.bb_id << ", lines: ";
    for(auto el : bbt.locations){
        os << el << ", ";
    }
    os << "\n";
    return os;
}

void addCost(sourcecost_t& sc, SourceLocation& sourceLoc, double cost){

  if(!sc.insert(std::pair<SourceLocation, double>(sourceLoc, cost)).second){
    sc[sourceLoc] = sc[sourceLoc] +  cost;
  }
}

double getTotalCost(const std::vector<BBTrace>& bbTvec,
		    const std::vector<BasicBlock*>& bbVec,
		    std::function<double (Instruction*)> cf){
  double result = 0;
  for(auto& bbt : bbTvec){
    auto bb = bbVec[bbt.getBBid()];
    for(auto& I : *bb){
      result += cf(&I);
    }
  }
  return result;
}

sourcecost_t getCost(const std::vector<BBTrace>& bbTvec, 
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
	auto locations = bbt.getLocations();
	for(auto loc_it = locations.rbegin();
	    loc_it != locations.rend(); loc_it++){
	  if(cg[curr_node].f->getName() == "main"){
	    addCost(sc, *loc_it, cost);
	  }
	  else if(checkIncrease(cg, rec_callsites, *loc_it, in_edge)){
	    addCost(sc, *loc_it, cost);
	  }
	  //move to next node in call graph
	  if(loc_it != locations.rend()){
	    in_edge = getNextEdge(cg, curr_node, *loc_it);
	    curr_node = target(in_edge, cg);
	  }
	}
      }
    }
  }
  return sc;
}

sourcecost_t getIC(const std::vector<BBTrace>& bbTvec, 
			  const std::vector<BasicBlock*>& bbVec,
			  const CallGraph& cg,
			  bool attributeCallsites){
  auto costF = [](Instruction* I){ return 1; };
  return getCost(bbTvec, bbVec, cg, costF, attributeCallsites);
}

double getTotalLLVM(const std::vector<BBTrace>& bbTvec,
			   const std::vector<BasicBlock*>& bbVec){
  auto cf = [](Instruction* I){ return 1; };
  return getTotalCost(bbTvec, bbVec, cf);
}

sourcecost_t getICAss(const std::vector<BBTrace>& bbTvec, 
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

double getTotalAss(const std::vector<BBTrace>& bbTvec, 
			     const std::vector<BasicBlock*>& bbVec,
			     std::map<Instruction*, unsigned long>&
			     instrIndex,
			     LinesInstr& linesAddr){
  auto cf = [&instrIndex, &linesAddr](Instruction* I){
    return linesAddr[instrIndex[I]].size();
  };
  
  return getTotalCost(bbTvec, bbVec, cf);
}

sourcecost_t getJoule(const std::vector<BBTrace>& bbTvec,
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
      cost += costMap.getCost(assInstr);
    }
    return cost;
  };

  return getCost(bbTvec, bbVec, cg, cf, attributeCallsites);
}

double getTotalJoule(const std::vector<BBTrace>& bbTvec,
			     const std::vector<BasicBlock*> bbVec,
			     std::map<Instruction*, unsigned long>&
			     instrIndex,
			     LinesInstr& linesAddr,
			     costMap_t costMap) {
  auto cf = [&instrIndex, &linesAddr, &costMap](Instruction* I){
    double cost = 0;
    for(auto& assInstr : linesAddr[instrIndex[I]]) {
      cost += costMap.getCost(assInstr);
    }
    return cost;
  };

  return getTotalCost(bbTvec, bbVec, cf);
}

sourcecost_t getCost(std::vector<BasicBlock*>& bbVec,
			    std::vector<uint64_t>& counts,
			    std::function<double (Instruction*)> costFunction){
  assert(bbVec.size() == counts.size() && "Size mismatch in count");
  sourcecost_t sc;
  for(int i = 0; i < bbVec.size(); i++){
    auto bb = bbVec[i];
    auto count = counts[i];
    errs() << count << "\n";
    SourceLocation funcLoc(bb->getParent()->getSubprogram());
    for(auto& I : *bb){
      auto cost = costFunction(&I);
      if(auto loc = I.getDebugLoc()){
	SourceLocation sourceLoc(loc.get());
	addCost(sc, sourceLoc, double(count)*cost);
      }
      else{
	addCost(sc, funcLoc, double(count)*cost);
      }
    }
  }

  return sc;
}

double getTotalCost(std::vector<BasicBlock*>& bbVec,
		    std::vector<uint64_t>& counts,
		    std::function<double (Instruction*)> costFunction){
  assert(bbVec.size() == counts.size() && "Size mismatch in count");
  double result = 0;
  
  for(auto p : zip(bbVec, counts)){
    auto& [bb, count] = p;
    for(auto& I : *bb){
      auto cost = costFunction(&I);
      result += double(count)*cost;
    }
  }
  return result;
}

//! Counts how many times each assembly instruction has been executed
map<string, unsigned long>
getInstrCount(const std::vector<BasicBlock*> bbVec,
	      vector<uint64_t> counts,
	      std::map<Instruction*, unsigned long>&
	      instrIndex,
	      LinesInstr& linesAddr){
  map<string, unsigned long> res;

  for(auto p : zip(bbVec, counts)){
    auto& [bb, count] = p;
    for(auto& I : *bb){
      auto objInstrs = linesAddr[instrIndex[&I]];
      for(auto& objI : objInstrs){
	if(!res.insert(make_pair(objI.getOperation(), count)).second){
	  res[objI.getOperation()] += count;
	}
      }
    }
    
  }
  return res;
}

map<pair<string, string>, unsigned long>
getPairCount(const vector<BasicBlock*> bbVec,
	     vector<uint64_t> counts,
	     map<Instruction*, unsigned long>& instrIndex,
	     LinesInstr& linesAddr,
	     const ObjModule& objM){
  map<pair<string, string>, unsigned long> res;
  for(auto p : zip(bbVec, counts)){
    auto& [bb, count] = p;
    for(auto& I : *bb){
      auto objInstrs = linesAddr[instrIndex[&I]];
      for(auto& objI : objInstrs){
	if(objM.isEndAddr(objI.getAddr()))
	  continue;
	auto nextI = objM.getInstrByAddr(objI.getAddr() + objI.getSize());
	auto entry = make_pair(objI.getOperation(), nextI.getOperation());
	if(!res.insert(make_pair(entry, count)).second){
	  res[entry] += count;
	}
      }
    }
  }

  return res;
}
