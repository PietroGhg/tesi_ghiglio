#include "costmap.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/Object/Binary.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include <boost/algorithm/string/predicate.hpp>

costMap_t getCostMap(string path) {
  fstream f(path);
  assert(f.is_open() && "no json file.");
  Json::Value root;
  f >> root;
  auto freq = root["freq"].asDouble();
  auto power = root["power"].asDouble();
  auto iiOver = root["iiover"].asDouble();
  auto cpiVec = root["cpi"];
  costMap_t cm(iiOver);

  for(auto& entry : cpiVec){
    auto name = entry["opname"].asString();
    double cpi = entry["cost"].asDouble();
    double cost = cpi*power/freq;
    cm.insert(name, cost);
  }

  return cm;
}

double CostMap::getCost(const ObjInstr& i){
  return operator[](i.getOperation()) + iiOver;
}

const double& CostMap::operator[](const string& name){
  auto it = cm.find(name);
  bool found = it != cm.end();
  if(found)
    return it->second;
  //else
  return fallback(name);
}

const double& CostMap::fallback(const string& name){
  /*if(boost::algorithm::starts_with(name, "v")){
    return operator[](name.substr(1, name.size()));    
  }*/
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

int STICost::getLatency(const MCInst& I){
  //Taken from llvm-mca
  const MCInstrDesc &MCDesc = MCII->get(I.getOpcode());
  const MCSchedModel &SM = STI->getSchedModel();

  // Obtain the scheduling class information from the instruction.
  unsigned SchedClassID = MCDesc.getSchedClass();
  unsigned CPUID = SM.getProcessorID();

  // Try to solve variant scheduling classes.
  while (SchedClassID && SM.getSchedClassDesc(SchedClassID)->isVariant())
    SchedClassID = STI->resolveVariantSchedClass(SchedClassID, &I, CPUID);

  const MCSchedClassDesc &SCDesc = *SM.getSchedClassDesc(SchedClassID);
  unsigned NumMicroOpcodes = SCDesc.NumMicroOps;
  int Latency = MCSchedModel::computeInstrLatency(*STI, SCDesc);
  // Add extra latency due to delays in the forwarding data paths.
  Latency += MCSchedModel::getForwardingDelayCycles(
    STI->getReadAdvanceEntries(SCDesc));

  return Latency;
}

double STICost::getCost(const MCInst& I){
  return power*(double)getLatency(I)/freq;
}

STICost::STICost(const MCSubtargetInfo* sti, const MCInstrInfo* mcii, double freq, double power):
  STI(sti), MCII(mcii), freq(freq), power(power){}

STICost initSTICost(const string& objPath, const string& jsonPath){
  using namespace object;
  //Open json
  fstream jsonFile(jsonPath);
  assert(jsonFile.is_open() && "file not open");
  Json::Value root;
  jsonFile >> root;
  //Get Object
  StringRef Filename(objPath);
  ErrorOr<std::unique_ptr<MemoryBuffer>> BuffOrErr =
    MemoryBuffer::getFileOrSTDIN(Filename);
  std::unique_ptr<MemoryBuffer> Buffer = std::move(BuffOrErr.get());
  Expected<std::unique_ptr<Binary>> BinOrErr = object::createBinary(*Buffer);
  ObjectFile* Obj;
  if(bool(BinOrErr)){
    Obj = dyn_cast<ObjectFile>(BinOrErr->get());
  }
  else{
    errs() << "Cannot create binary\n";
  }

  //Get Target
  string Error;
  auto theTriple = Obj->makeTriple();
  InitializeAllTargetInfos();
  InitializeAllTargetMCs();
  InitializeAllDisassemblers();
  const Target* theTarget =
    TargetRegistry::lookupTarget(theTriple.getTriple(), Error);

  //Retrieve STI
  string MCPU = root["name"].asString();
  errs() << "CPU: " << MCPU << "\n";
  SubtargetFeatures Features = Obj->getFeatures();
  const MCSubtargetInfo* STI =
    theTarget->createMCSubtargetInfo(theTriple.getTriple(),
				     MCPU,
				     Features.getString());
 
  assert(STI && "no STI");
  if(!STI->getSchedModel().hasInstrSchedModel()){
    errs() << "No scheduling model for " << theTriple.normalize() << " and CPU " << MCPU << "\nExiting.\n";
    exit(1);
  }

  //Retrieve MCII
  const MCInstrInfo* MCII = theTarget->createMCInstrInfo();
  assert(MCII && "no mcinstrinfo!");

  //Retrieve freq and power from json
  auto freq = root["freq"].asDouble();
  auto power = root["power"].asDouble();

  return STICost(STI, MCII, freq, power);
}
