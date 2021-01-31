#pragma once
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/StringExtras.h"	//arrayRefFromStringRef
#include "llvm/ADT/StringRef.h"
#include "llvm/DebugInfo/DIContext.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/DWARF/DWARFDebugLine.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/TargetRegistry.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include <bits/stdint-uintn.h>
#include <boost/algorithm/string/trim.hpp>
#include "boost/algorithm/string.hpp" //string split, trim


// targets for disassembly
#include "llvm/Support/TargetSelect.h"

using namespace llvm;
using namespace object;

/**This classes are used to retrieve the id of an llvm instruction
 * found in the debug info of an assembly file whose line information in the line table
 * have been replaced with the instruction id
 */
using llvmID = unsigned long;
//! map from an address to llvm id
using AddrLines = std::map<uint64_t, llvmID>;
//! map from an llvmID to all the addresses associated to it
using LinesAddr = std::map<llvmID, std::vector<uint64_t>>;




class ObjInstr{
private:
  uint64_t addr;
  uint64_t size;
  llvm::MCInst inst;
  std::string disass;
  llvmID debugLoc = 0; // 0 -> no debugLoc
public:
  ObjInstr(uint64_t _addr, uint64_t _size,
	   llvm::MCInst& _inst,
	   std::string& _disass):
    addr(_addr), size(_size), inst(_inst), disass(_disass){}
  uint64_t getAddr() const { return addr; }
  uint64_t getSize() { return size; }
  void setDebugLoc(llvmID _debugLoc) { debugLoc = _debugLoc; }
  bool hasDebugLoc(){ return debugLoc != 0; }
  llvmID getDebugLoc() const { return debugLoc; }

  
  bool operator<(const ObjInstr& other) const {
    return getAddr() < other.getAddr();
  }

  std::string getOperation() const {
    auto disass_copy = boost::trim_left_copy(disass);
    std::vector<std::string> splitted;
    boost::algorithm::split(splitted, disass_copy,
			    [](char c){ return c == ' ' || c == '\t';});
    return splitted[0];
  }

  const MCInst& getMCInstr() const {
    return inst;
  }

  void dump(){
    errs() << "addr: ";
    errs().write_hex(addr) << " ";
    errs() << disass << " ";
    if(debugLoc){
      errs() << "debug: " << debugLoc <<  " ";
    }
    errs() << "\n";
  }
};

class ObjFunction {
private:
  std::string name;
  uint64_t begin;
  std::vector<ObjInstr> instructions;
public:
  ObjFunction(std::string _name):
    name(_name){}
  std::string getName() const { return name; }
  uint64_t getBegin() const { return begin; }
  uint64_t getEnd() const { return (instructions.end() - 1)->getAddr(); }
  void setBegin(uint64_t _begin){ begin = _begin; }
  void addInst(ObjInstr inst){ instructions.push_back(inst); }
  const std::vector<ObjInstr>& getInstructions() const {
    return instructions;
  }

  std::vector<ObjInstr>& getInstructions(){
    return instructions;
  }

  void setDebugLocations(const AddrLines& addrs){
    for(auto& instr : instructions){
      auto toFind = addrs.find(instr.getAddr());
      if(toFind != addrs.end()){
	instr.setDebugLoc(toFind->second);
      }
    }
  }

  /**
   * Completes the debug locations found in the line table, by 
   * associating each instructrion with unknown location to the 
   * previous known location.
   */
  void completeDebugLoc(){
    llvmID loc;
    for(auto& i : instructions){
      if(i.hasDebugLoc()){
	loc = i.getDebugLoc();
      }
      else{
	i.setDebugLoc(loc);
      }
    }
  }

  /**
   * LLVM automatically maps the function prologue to to the source line
   * in which the function has been defined.
   * This leads to errors in the attribution.
   */
  void fixPrologue(){
    bool found_once = false;
    llvmID loc;
    int index;
    for(index = 0; index < instructions.size(); index++){
      if(instructions[index].hasDebugLoc() && !found_once){
	found_once = true;
      }
      else if(instructions[index].hasDebugLoc() && found_once){
	loc = instructions[index].getDebugLoc();
	index++;
	break;
      }
    }

    for(int i = 0; i < index; i++){
      instructions[i].setDebugLoc(loc);
    }
  }

  const ObjInstr& getInstrByAddr(unsigned long addr) const {
    for(auto& i : instructions){
      if(i.getAddr() == addr)
	return i;
    }
    assert(false && "no addr in ObjFunction");
  }
      
    
  void dump(){
    errs() << "name: " << name << "\n";
    errs() << "begin: ";
    errs().write_hex(begin) << " end: ";
    errs().write_hex(getEnd()) << "\n";
    for(auto i : instructions){
      i.dump();
    }
  }

};

using InstrLines = std::map<ObjInstr, llvmID>;
using LinesInstr = std::map<llvmID, std::vector<ObjInstr>>;

class ObjModule {
private:
  std::vector<ObjFunction> functions;
public:
  void addFunction(ObjFunction function){
    functions.push_back(std::move(function));
  }

  std::vector<ObjFunction>& getFunctions(){
    return functions;
  }

  void setDebugLocations(const AddrLines& addrs){
    for(auto& el : functions){
      el.setDebugLocations(addrs);
    }
  }

  void completeDebugLoc(){
    for(auto& el : functions){
      el.completeDebugLoc();
    }
  }
  
  void fixPrologues(){
    for(auto& el : functions){
      el.fixPrologue();
    }
  }
  
  void dump() const {
    for(auto f : functions){
      f.dump();
    }
  }

  bool isEndAddr(unsigned long addr) const {
    for(auto& f : functions){
      if(addr == f.getEnd())
	return true;
    }
    return false;
  }

  const ObjInstr& getInstrByAddr(unsigned long addr) const {
    for(auto& f : functions){
      if(addr >= f.getBegin() && addr <= f.getEnd())
	return f.getInstrByAddr(addr);
    }
    errs() << "addr: " << addr << "\n";
    assert(false && "no addr in module");
  }

  InstrLines getMap() const {
    InstrLines res;
    for(const auto& f : functions){
      for(const auto& i : f.getInstructions()){
	res[i] = i.getDebugLoc();
      }
    }
    return res;
  }
};

//! Returns names of the functions in a Module
std::vector<std::string> funcNames(const llvm::Module& m);

//! Returns the address of the .text section of the binary
llvm::object::SectionRef getDotText(const llvm::object::ObjectFile& obj);

//! Returns the end address of the section identified by currS
uint64_t getEnd(const std::vector<llvm::object::SymbolRef>& symbols,
		       std::vector<llvm::object::SymbolRef>::const_iterator currSym);
  

//! Returns the symbols contained in the .text section of the binary, sorted by address
std::vector<llvm::object::SymbolRef> getTextSymbols(const llvm::object::ObjectFile& obj);
    
//! Returns the map between binary insturctions and source line, note that the source line will be replaced with the identifier of the llvm instruction
AddrLines getAddrLines(llvm::DWARFContext& DCtx);

//! Transposes an AddrLines object, returning the map between source line and all the binary instructions corresponding to it
LinesInstr getLinesAddr(const InstrLines& addrs);

//! Attemps to disassemble the function identified by name, pair::second is true if the disassembling succeded.
std::pair<ObjFunction,bool>
getFun(const std::vector<llvm::object::SymbolRef>& symbols,
       const std::string& name,
       llvm::MCInstPrinter& ip,
       const llvm::MCDisassembler& DisAsm,
       const llvm::MCSubtargetInfo& sti,
       const llvm::ArrayRef<uint8_t>& bytes);

//! Returns the line -> [addresses] map
LinesInstr getMap(const std::string& objPath,
		  const std::string& MCPU,
		  const Module& m,
		  const bool printDisAss);

//! Returns the ObjModule corresponding to the Object file
ObjModule getObjM(const std::string& objPath,
		  const std::string& MCPU,
		  const Module& m,
		  const bool printDisAss);
