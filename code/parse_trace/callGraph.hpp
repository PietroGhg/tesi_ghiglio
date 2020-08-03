#pragma once
#include <boost/graph/adjacency_list.hpp>
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
};

typedef adjacency_list<vecS, 
		       vecS,
		       bidirectionalS,
		       VertexData,
		       EdgeData> CallGraph;

using nodemap_t= std::map<Function*, int>;

nodemap_t makeNodeMap(Module* M){
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

CallGraph makeCallGraph(Module* M){
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
                            errs() << "no debug locations\n";
                        }
                    }
                }
            }
        }
    }
    return std::move(res);
}

void printCG(CallGraph& cg){
    for(auto v : make_iterator_range(vertices(cg))){
        errs() << cg[v].f->getName() << ": ";
        for(auto e : make_iterator_range(out_edges(v, cg))){
            errs() << cg[target(e, cg)].f->getName() << " at location: " << cg[e].callsite << ", ";
        }
        errs() << "\n";
    }
}