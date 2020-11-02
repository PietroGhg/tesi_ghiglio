#pragma once
#include <boost/graph/adjacency_list.hpp>
#include "llvm/IR/Module.h"
#include <llvm/IR/Function.h>
#include "llvm/IR/Instructions.h" //CallInst
#include "llvm/IR/IntrinsicInst.h" //DBGInfoIntrinsic
#include <map>

using namespace llvm;
using namespace boost;

struct VertexData {
    Function* f;
};

using callsite_t = unsigned int;

struct EdgeData {
    callsite_t callsite;
    bool recursive;
};

/** Data type for a graph that represents the function calls at source code level:
 * each node corresponds to a function
 * each edge corresponds to a callsite in the source code, labeled with the callsite information.
 */
typedef adjacency_list<vecS, 
		       vecS,
		       bidirectionalS,
		       VertexData,
		       EdgeData> CallGraph;
using edge_t = boost::graph_traits<CallGraph>::edge_descriptor;
using vertex_t = boost::graph_traits<CallGraph>::vertex_descriptor;

//! data type for the map from a function to its vertex descriptor in the call graph
using nodemap_t = std::map<Function*, vertex_t>;

//! function that creates an entry in the map for each function in the module
inline nodemap_t makeNodeMap(Module* M){
    nodemap_t res;
    int i = 0;
    for(auto& f : *M){
        if(f.isDeclaration())
            continue;
        
        res[&f] = i;
        i++;
    }
    return std::move(res);
}

/**function that checks if the function call corresponding to the given edge leads
 * to a call to the function correspoding to the source.
 */
inline bool isRecursive(CallGraph& cg, edge_t edge, vertex_t source, std::set<edge_t> edges){
    if(target(edge,cg) == source){
        return true;
    }
    bool is_rec = false;
    auto tgt = target(edge,cg);
    for(auto e : make_iterator_range(out_edges(tgt, cg))){
        if(edges.insert(e).second){
            is_rec = is_rec || isRecursive(cg, e, source, edges);
        }
        else{
            return false;
        }
    }
    return is_rec;
}

/** function that sets the recursive attribute in the edges of the call graph
 */
inline void setRecursive(CallGraph& cg){
    for(auto e : make_iterator_range(edges(cg))){
        std::set<edge_t> explored_edges;
        explored_edges.insert(e);
        cg[e].recursive = isRecursive(cg, e, source(e,cg), explored_edges);
    }
}

//! Function that builds a call graph by inspecting all the CallInst in a module
inline CallGraph makeCallGraph(Module* M){
    auto nm = makeNodeMap(M);
    CallGraph res(nm.size());
    for(auto el : nm){
        res[el.second].f = el.first;
    }
    

    for(auto& f : *M){
        if(f.isDeclaration())
            continue;
        
        for(auto& bb : f){
            for(auto& i : bb){
                if(isa<CallInst>(i) && !isa<DbgInfoIntrinsic>(i)){
                    auto call_i = dyn_cast<CallInst>(&i);
                    auto called = call_i->getCalledFunction();
                    if(!called->isDeclaration()){
                        auto e = add_edge(nm[&f], nm[called], res);
                        if(auto loc = i.getDebugLoc()){
                            res[e.first].callsite = loc.getLine();
                        }
                        else{
                            errs() << "no debug location for instruction " << i << "\n";
                        }
                    }
                }
            }
        }
    }
    setRecursive(res);
    return std::move(res);
}

inline void printCG(const CallGraph& cg){
    for(auto v : make_iterator_range(vertices(cg))){
        errs() << cg[v].f->getName() << ": ";
        for(auto e : make_iterator_range(out_edges(v, cg))){
            errs() << cg[target(e, cg)].f->getName() << " loc: " << cg[e].callsite;
            errs() << " rec: " << cg[e].recursive << " ";
        }
        errs() << "\n";
    }
}

inline std::string vertDOT(const CallGraph& cg, vertex_t v){
  return cg[v].f->getName().str();
}

inline std::string edgeDOT(const CallGraph& cg, edge_t e){
    auto s = vertDOT(cg, source(e,cg));
    auto t = vertDOT(cg, target(e,cg));
    auto f = [](bool rec){ return rec ? "rec" : "not_rec";};
    return s + " -> " + t + "[label = \"" + f(cg[e].recursive) + " line: " + std::to_string(cg[e].callsite) + "\"]";
}

inline std::string getDOT(const CallGraph& cg){
    std::string res = "";
    res += "digraph cg {\n";
    for(auto v : make_iterator_range(vertices(cg))){
        res += vertDOT(cg, v) + "\n";
    }
    for(auto e : make_iterator_range(edges(cg))){
        res += edgeDOT(cg, e) + "\n";
    }
    res += "}";
    return std::move(res);
}

inline vertex_t getMainNode(const CallGraph& cg){
    vertex_t res;
    bool found = false;
    for(auto v : make_iterator_range(vertices(cg))){
        if(cg[v].f->getName() == "main"){
            return v;
        }
    }

    errs() << "no main function\n";
    throw;
}

//! Function that returns the edge correspoding to the callsite given as parameter
inline edge_t getNextEdge(const CallGraph& cg, const size_t curr_node, const callsite_t callsite){
    edge_t res;
    for(auto e : make_iterator_range(out_edges(curr_node, cg))){
        if(cg[e].callsite == callsite){
            return e;
        }
    }
    errs() << "error while computing next node\n";
    errs() << "from node: " << cg[curr_node].f->getName();
    errs() << " callsite: " << callsite << "\n";
    throw;
}

//! Function that returns true if the given callsite corresponds to a recursive call
inline bool isRecursive(const CallGraph& cg, callsite_t line, const std::set<edge_t>& rec_callsites){
    for(auto el : rec_callsites){
        if(cg[el].callsite == line){
            return true;
        }
    }
    return false;
}

//! Function that returns the edge correspoding to a callsite, only if that callsite is recursive
inline edge_t getEdge(const CallGraph& cg, const std::set<edge_t>& rec_callsites, callsite_t line){
    for(auto el : rec_callsites){
        if(cg[el].callsite == line){
            return el;
        }
    }

    errs() << "error while returning edge of a recursive callsite.\n";
    errs() << "is it really recursive?\n";
    throw;
}

//! Function that checks wheter to assign or not the instruction count cost to a given callsite
inline bool checkIncrease(const CallGraph& cg, const std::set<edge_t>& rec_callsites, callsite_t line, edge_t in_edge){
    return !isRecursive(cg, line, rec_callsites);
}

inline std::set<edge_t> getRecursiveCallSites(const CallGraph& cg){
    std::set<edge_t> res;
    for(auto e : make_iterator_range(edges(cg))){
        if(cg[e].recursive){
            res.insert(e);
        }
    }
    return std::move(res);
}

inline edge_t getEdgeByCallsite(CallGraph& cg, callsite_t callsite){
    for(auto e : make_iterator_range(edges(cg))){
        if(cg[e].callsite == callsite){
            return e;
        }
    }

    errs() << "error while retrieving edge by callsite\n";
    throw;
}
