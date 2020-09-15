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
#include "callGraph.hpp"
#include "map.hpp"
using namespace llvm;

/**
 * Class for the trace left by the execution of a single basic block
 */
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

raw_ostream& operator<<(raw_ostream& os, const BBTrace& bbt){
    os << "bb_id: " << bbt.bb_id << ", lines: ";
    for(auto el : bbt.lines){
        os << el << ", ";
    }
    os << "\n";
    return os;
}

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

/**
 * Function that computes the instruction count of each source code line
 * Returns a vector s.t. at position i there's the instruction count associated to line i
 */
std::vector<int> getIC(int num_lines, const std::vector<BBTrace>& bbTvec, 
                       const std::vector<BasicBlock*>& bbVec,
                       const CallGraph& cg){
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
                ic[line]++;
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
    return std::move(ic);
}

std::vector<int> getICAss(int num_lines, const std::vector<BBTrace>& bbTvec, 
			  const std::vector<BasicBlock*>& bbVec,
			  const CallGraph& cg,
			  std::map<Instruction*, unsigned long> instrIndex,
			  LinesAddr& linesAddr){
  //TODO: this should carry also info about which function the line belongs to
  //it may be found in debug info of module
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
	ic[line] = ic[line] + linesAddr[instrIndex[&i]].size(); //TODO: not + 1 but + count[i].
      }
      else{
	ic[funcLine] = ic[funcLine] + linesAddr[instrIndex[&i]].size(); //TODO: not + 1 but + count[i].
	//unass[i.getParent()->getParent()]++;
      }
      
      for(auto line_it = lines.rbegin(); line_it != lines.rend(); line_it++){
	if(cg[curr_node].f->getName() == "main"){
	  ic[*line_it] = ic[*line_it] + linesAddr[instrIndex[&i]].size();
	}
	else if(checkIncrease(cg, rec_callsites, *line_it, in_edge)){
	  ic[*line_it] = ic[*line_it] + linesAddr[instrIndex[&i]].size();
	}
	//move to next node in call graph
	if(line_it != lines.rend() - 1){
	  in_edge = getNextEdge(cg, curr_node, *std::next(line_it));
	  curr_node = target(in_edge, cg);
	}
      }
    }
  }
  return std::move(ic);
}

//1 -> original module .ll
//2 -> trace
//3 -> source .c
//4 -> executable with replaced debug info
//5 -> replaced module 
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
    std::ifstream source(argv[3]);
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



    auto theMap = getMap(argv[4], *m);

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
