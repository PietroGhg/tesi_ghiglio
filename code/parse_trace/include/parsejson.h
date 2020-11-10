#include <json/json.h>
#include <fstream>
#include <assert.h>
#include <map>

using namespace std;
using costMap_t = map<string, double>;

inline costMap_t getCostMap(double freq, double power) {
  costMap_t cm;
  fstream f("../cpi.json");
  assert(f.is_open() && "file not open");
  Json::Value root;
  f >> root;

  for(int i = 0; i < root.size(); i++){
    cm.insert(pair<string,int>(root[i]["opname"].asString(),
			       root[i]["cost"].asDouble()*freq*power));
  }

  return cm;
}
