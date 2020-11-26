#include "extendCall.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

ExtendedBB::ExtendedBB(unsigned long id, BasicBlock* BB):
  currCallSite(0), id(id) {
  //push callsites in the vector
  for(auto& I : *BB){
    if(isa<CallInst>(I) && I.getDebugLoc() && !isa<DbgInfoIntrinsic>(I)){
      auto CI = dyn_cast<CallInst>(&I);
      //check if the call is to a function in the CU
      auto called = CI->getCalledFunction();
      if(!called->isDeclaration()){
	SourceLocation loc(CI->getDebugLoc());
	callSites.push_back(loc);
      }
    }
  }

  //set isReturn
  auto termInst = BB->getTerminator();
  isReturnBB = isa<ReturnInst>(termInst);
}

void CallSitesStack::push(StackElement el){
  stack.push_back(el);
}

/**
 * Pops an element from the callsite stack.
 * Check if there are other callsites that must be pushed.
 * If the element was a return-terminated bb, recursively pop the next element
 * 
 */
void CallSitesStack::pop(){
  if(stack.empty()){
    return;
  }
  auto lastEl = stack.back();
  ExtendedBB* extBB = lastEl.getExtBB();
  assert(extBB->hasCallSites() && "trying to pop bb with no callsites");

  if(extBB->finished()){
    stack.pop_back();
    if(extBB->isReturn()){
      pop();
    }
  }
  else{
    stack.pop_back();
    extBB->increaseCurrCS();
    stack.emplace_back(extBB, extBB->getCurrCS());
  }
}

BBTrace CallSitesStack::getBBT(unsigned long id){
  vector<SourceLocation> locations;
  for(auto it = stack.rbegin(); it != stack.rend(); it++){
    locations.push_back(it->getCallSite());
  }

  return BBTrace(id, locations);
}

/**
 * Function that, given a trace of extended basic blocks, 
 * returns a trace equivalent to the output of the old instrumentation
 */
vector<BBTrace> extendBBT(vector<ExtendedBB>& extBBs){
  vector<BBTrace> result;
  CallSitesStack csStack;

  for(auto& extBB : extBBs){
    if( !extBB.hasCallSites() && !extBB.isReturn()){
      //just add corresponding bbt
      result.push_back(csStack.getBBT(extBB.getID()));
    }
    else if(!extBB.hasCallSites() && extBB.isReturn()){
      //add bbt
      result.push_back(csStack.getBBT(extBB.getID()));
      //pop callsite
      csStack.pop();
    }
    else if(extBB.hasCallSites()){
      //add bbt
      result.push_back(csStack.getBBT(extBB.getID()));
      //push a stack element
      csStack.push(StackElement(&extBB, extBB.getCallSites()[0]));
    }
  }

  return result;
}


vector<BBTrace> getSimpleBBTVec(const string& path){
  vector<BBTrace> result;
    ifstream f(path);
    string line;
    assert(f.is_open() && "error while opening trace file\n");

    while(std::getline(f, line)){
      result.push_back(BBTrace(line));
    }
    f.close();
    return result;  
}

vector<ExtendedBB> getExtBBVec(const vector<BasicBlock*>& bbVec,
			       const vector<BBTrace>& simpleBBT){
  vector<ExtendedBB> result;
  for(auto& bbt : simpleBBT){
    result.emplace_back(bbt.getBBid(), bbVec[bbt.getBBid()]);
  }

  return result;
}
