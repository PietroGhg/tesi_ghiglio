#pragma once
#include <json/json.h>
#include <fstream>
#include <assert.h>
#include <map>
#include "llvm/Support/raw_ostream.h"

using namespace std;
using costMap_t = map<string, double>;

inline costMap_t getCostMap(double freq, double power, string path) {
  costMap_t cm;
  fstream f(path);
  assert(f.is_open() && "file not open");
  Json::Value root;
  f >> root;

  for(int i = 0; i < root.size(); i++){
    auto name = root[i]["opname"].asString();
    double cpi = stod(root[i]["cost"].asString());
    double cost = cpi*power/freq;
    auto check = cm.insert(pair<string,int>(name, cost));
    assert(check.second && "element already existed in cost map");
  }

  return cm;
}
