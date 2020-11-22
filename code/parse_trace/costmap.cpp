#include "costmap.h"
#include <boost/algorithm/string/predicate.hpp>

double& CostMap::operator[](const string& name){
  auto it = cm.find(name);
  bool found = it != cm.end();
  if(found)
    return it->second;
  //else
  return fallback(name);
}

double& CostMap::fallback(const string& name){
  if(boost::algorithm::ends_with(name, "s")){
    //sets flag operation
    return operator[](name.substr(0, name.size() - 1));
  }
  if(isConditional(name)){
    //conditional operation
    return operator[](name.substr(0, name.size() - 2));
  }
  if(boost::algorithm::ends_with(name, ".w")){
    //wide immediate
    return operator[](name.substr(0, name.size() - 2));
    
  }
  llvm::errs() << "Cannot find " << name << "\nExiting.\n";
  exit(-1);
}

bool CostMap::isConditional(const string& name){
  for(auto& suff : condSuffixes){
    if(boost::algorithm::ends_with(name, suff)){
      return true;
    }
  }
  return false;
}
