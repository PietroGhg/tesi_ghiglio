#include <json/json.h>
#include <fstream>
#include <assert.h>
#include <map>

using namespace std;
using costMap_t = map<string, string>;

inline costMap_t test() {
  costMap_t cm;
  fstream f("../cpi.json");
  assert(f.is_open() && "file not open");
  Json::Value root;
  f >> root;

  for(int i = 0; i < root.size(); i++){
    cm.insert(pair<string,string>(root[i]["opname"].asString(),
				  root[i]["cost"].asString()));
  }

  return cm;
}
