#pragma once
#include "boost/algorithm/string.hpp" //string split
#include "llvm/Support/raw_ostream.h"
#include "callGraph.hpp"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DebugInfoMetadata.h" //DISubprogram
#include <functional> //std::function
#include "map.hpp"

using namespace llvm;

class BBTrace{
private:
    int bb_id;
    std::vector<int> lines;
public:
    //assumes to receive a string in the shape: bb_id line1 line2 line3
    BBTrace(std::string trace){
        std::vector<std::string> splitted;
        boost::algorithm::split(splitted, trace, [](char c){ return c == ' ';});
        bb_id = std::stoi(splitted[0]);
        std::transform(splitted.begin()+1, splitted.end(), std::back_inserter(lines), 
                        [](std::string s){ return std::stoi(s); });
    }
    int getBBid() {return bb_id; }
    std::vector<int> getLines(){ return lines; }
    friend raw_ostream& operator<<(raw_ostream& os, const BBTrace& bbt);

};

inline raw_ostream& operator<<(raw_ostream& os, const BBTrace& bbt){
    os << "bb_id: " << bbt.bb_id << ", lines: ";
    for(auto el : bbt.lines){
        os << el << ", ";
    }
    os << "\n";
    return os;
}

inline std::vector<int> getICgen(int num_lines,
				 const std::vector<BBTrace>& bbTvec, 
				 const std::vector<BasicBlock*>& bbVec,
				 const CallGraph& cg,
				 std::function<unsigned (Instruction*)>
				 costFunction){
    //TODO: this should carry also info about which function the line belongs to
    std::vector<int> ic(num_lines+1); 
    auto main_node = getMainNode(cg);
    auto rec_callsites = getRecursiveCallSites(cg);
    vertex_t curr_node = main_node;
    edge_t in_edge;

    for(auto bbt : bbTvec){
        for(auto& i : *bbVec[bbt.getBBid()]){
	  auto funcLine = i.getParent()->getParent()->getSubprogram()->getLine();
            curr_node = main_node;
            auto loc = i.getDebugLoc();
            auto lines = bbt.getLines();
            if(loc){
                auto line = loc.getLine();
		if(line < ic.size()) //TODO: remove this after fix of instrumentation
		  ic[line] += costFunction(&i);
            }
            else{
	      ic[funcLine]++;
            }
            
            for(auto line_it = lines.rbegin(); line_it != lines.rend(); line_it++){
                if(cg[curr_node].f->getName() == "main"){
                    ic[*line_it]++;
                }
                else if(checkIncrease(cg, rec_callsites, *line_it, in_edge)){
                    ic[*line_it]++;
                }
                //move to next node in call graph
                if(line_it != lines.rend() - 1){
                    in_edge = getNextEdge(cg, curr_node, *std::next(line_it));
                    curr_node = target(in_edge, cg);
                }
            }
        }
    }
    return ic;
}

inline std::vector<int> getIC(int num_lines, const std::vector<BBTrace>& bbTvec, 
                       const std::vector<BasicBlock*>& bbVec,
                       const CallGraph& cg){
  auto costF = [](Instruction* I){ return 1; };
  return getICgen(num_lines, bbTvec, bbVec, cg, costF);
}

inline std::vector<int> getICAss(int num_lines,
				 const std::vector<BBTrace>& bbTvec, 
				 const std::vector<BasicBlock*>& bbVec,
				 const CallGraph& cg,
				 std::map<Instruction*, unsigned long>&
				 instrIndex,
				 LinesAddr& linesAddr){
  auto cf = [&instrIndex, &linesAddr](Instruction* I){
    return linesAddr[instrIndex[I]].size();
  };
  
  return getICgen(num_lines, bbTvec, bbVec, cg, cf);
}
