#pragma once
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
  double& fallback(const string& name);
public:
  
  void insert(string name, double cost){
    auto check = cm.insert(pair<string,int>(name, cost));
    assert(check.second && "element already existed in cost map");
  }
  
  double& operator[](const string& name);
  
};

using costMap_t = CostMap;

inline costMap_t getCostMap(string path) {
  costMap_t cm;
  fstream f(path);
  assert(f.is_open() && "file not open");
  Json::Value root;
  f >> root;
  auto freq = root["freq"].asDouble();
  auto power = root["power"].asDouble();
  auto cpiVec = root["cpi"];

  for(auto& entry : cpiVec){
    auto name = entry["opname"].asString();
    double cpi = entry["cost"].asDouble();
    double cost = cpi*power/freq;
    cm.insert(name, cost);
  }

  return cm;
}

class STICost {
private:
  unique_ptr<const MCSubtargetInfo> STI;
  unique_ptr<const MCInstrInfo> MCII;
  double freq;
  double power;
public:
  STICost(const MCSubtargetInfo* sti, const MCInstrInfo* mcii, double freq, double power);
  unsigned getLatency(const MCInst& I);
  double getCost(const MCInst& I);
};

STICost initSTICost(const string& objPath, const string& jsonPath);
