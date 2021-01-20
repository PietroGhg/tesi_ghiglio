#pragma once
#include "map.h"
#include "map.h"
#include <json/json.h>
#include <fstream>
#include <assert.h>
#include <map>
#include <memory>
#include <vector>
#include "llvm/MC/MCSchedule.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/raw_ostream.h"
#include <boost/algorithm/string/predicate.hpp> //endswith

using namespace std;
using namespace llvm;

class CostMap {
private:
  const vector<string> condSuffixes{"eq", "ne", "cs", "hs", "cc", "lo",
      "mi", "pl", "vs", "vc", "hi", "ls", "ge", "lt", "gt", "le", "al"};
  map<string,double> cm;
  bool isConditional(const string& name);
  const double& fallback(const string& name);
  double iiOver;
public:
  CostMap(double iiover): iiOver(iiover){};
  void insert(string name, double cost){
    auto check = cm.insert(make_pair(name, cost));
    assert(check.second && "element already existed in cost map");
  }
  
  const double& operator[](const string& name);
  double getCost(const ObjInstr& i);
  double getInterInstOverhead() const {
    return iiOver;
  }
  
};

using costMap_t = CostMap;

costMap_t getCostMap(string path);

class STICost {
private:
  unique_ptr<const MCSubtargetInfo> STI;
  unique_ptr<const MCInstrInfo> MCII;
  double freq;
  double power;
public:
  STICost(const MCSubtargetInfo* sti, const MCInstrInfo* mcii, double freq, double power);
  int getLatency(const MCInst& I);
  double getCost(const MCInst& I);
};

STICost initSTICost(const string& objPath, const string& jsonPath);
