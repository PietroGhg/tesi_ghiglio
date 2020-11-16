#pragma once
#include <vector>
#include "llvm/IR/BasicBlock.h"
#include "sourcelocation.h"
#include "getIC.h"

using namespace llvm;
using namespace std;

/**
 * Class that represents a basic block and explicitly carries 
 * information about the calls in the bb, and wheter or not 
 * it terminates with a return instruction.
 */
class ExtendedBB {
 private:
  unsigned long id;
  int currCallSite;
  vector<SourceLocation> callSites;
  bool isReturnBB;
 public:
  ExtendedBB(unsigned long id, BasicBlock* BB);
  const vector<SourceLocation>& getCallSites() const { return callSites; }
  bool isReturn() const { return isReturnBB; }
  bool hasCallSites() const { return !callSites.empty(); }
  void increaseCurrCS() { currCallSite++; }
  bool finished() const { return currCallSite == (callSites.size() - 1); }
  SourceLocation getCurrCS() const {
    assert(currCallSite < callSites.size() && "currCallSite out of bounds");
    return callSites[currCallSite];
  }
  unsigned long getID() { return id; }
};

/**
 * Class for elements of the stack, carries the callsite and a pointer to the
 * basic block it came from.
 */
class StackElement {
 private:
  ExtendedBB* extBB;
  SourceLocation callSite;
 public:
 StackElement(ExtendedBB* extBB, SourceLocation callSite):
  extBB(extBB), callSite(callSite){};
  ExtendedBB* getExtBB() { return extBB; }
  SourceLocation getCallSite(){ return callSite; }
};

/**
 * Class for the stack of callsites
 */
class CallSitesStack {
 private:
  vector<StackElement> stack;
 public:
  void push(StackElement el);
  void pop();
  BBTrace getBBT(unsigned long id);
};

vector<BBTrace> getSimpleBBTVec(const string& path);
vector<ExtendedBB> getExtBBVec(const vector<BasicBlock*>& bbVec,
			       const vector<BBTrace>& simpleBBT);
vector<BBTrace> extendBBT(vector<ExtendedBB>& extBBs);


