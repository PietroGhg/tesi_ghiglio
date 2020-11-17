#pragma once
#include <json/json.h>
#include <fstream>
#include <assert.h>
#include <map>
#include "llvm/Support/raw_ostream.h"

using namespace std;
using costMap_t = map<string, double>;

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
    auto check = cm.insert(pair<string,int>(name, cost));
    assert(check.second && "element already existed in cost map");
  }

  return cm;
}
