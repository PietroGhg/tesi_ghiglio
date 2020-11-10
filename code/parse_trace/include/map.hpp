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
#include <iostream>

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

  void dump(){
    errs() << "addr: ";
    errs().write_hex(addr) << " ";
    errs() << disass << " ";
    //std::cout <<  disass << " ";
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
  
  void dump(){
    for(auto f : functions){
      f.dump();
    }
  }

  AddrLines getMap(){
    AddrLines res;
    for(const auto& f : functions){
      for(const auto& i : f.getInstructions()){
	res[i.getAddr()] = i.getDebugLoc();
      }
    }
    return res;
  }
};

inline bool isDotText(const llvm::object::SectionRef section){
  auto name = section.getName();
  return bool(name) && name.get() == ".text";
}

//TODO: function mangling
//http://www.avabodh.com/cxxin/namemangling.html
inline std::vector<std::string> funcNames(const llvm::Module& m){
  std::vector<std::string> res;
  for(auto& f : m){
    if(!f.isDeclaration())
      res.push_back(f.getName().str());
  }
  return res;
}

inline llvm::object::SectionRef getDotText(const llvm::object::ObjectFile& obj){
  for(auto sec : obj.sections()){
    if(isDotText(sec)){
      return std::move(sec);
    }
  }
  assert(false && "No .text");
}

inline uint64_t getEnd(const std::vector<llvm::object::SymbolRef>& symbols,
		       std::vector<llvm::object::SymbolRef>::const_iterator currSym){

  auto currAddr = currSym->getAddress();
  assert(bool(currAddr) && "no address in function start sym");
  auto nextSym = std::next(currSym);
  auto nextAddr = nextSym->getAddress();
  assert(bool(nextAddr) && "no address while computing function end");

  //need to iterate in order to avoid some symbols that have the same address
  //as the function start
  while(nextSym != symbols.end() && nextAddr.get() == currAddr.get()){
      nextSym = std::next(nextSym);
      nextAddr = nextSym->getAddress();
      assert(bool(nextAddr) && "no address while computing function end");
  }
  if(nextSym != symbols.end()){
    return nextAddr.get();
  }
  else{
    auto section = currSym->getSection();
    assert(bool(section) && "no section found while disassembling");
    return section.get()->getAddress() + section.get()->getSize();
  }
}
  

inline ObjFunction getFun(const std::vector<llvm::object::SymbolRef>& symbols,
			  const std::string& name,
			  const llvm::MCDisassembler& DisAsm,
			  llvm::MCInstPrinter& ip,
			  const llvm::MCSubtargetInfo& sti,
			  const llvm::ArrayRef<uint8_t>& bytes){
  ObjFunction res(name);
  auto it = symbols.begin();

  //find the iterator of the symbol corresponding to the given name
  for( ; it != symbols.end(); it++){
    auto symName = it->getName();
    assert(bool(symName) && "current symbol has no name");
    if(symName.get() == name){
      break;
    }
  }
  assert(it != symbols.end() && "symbol name not found");
  
  auto addr = it->getAddress();
  assert(bool(addr) && "current symbols has no address");
  //compute end of function: either next symbol or next section
  res.setBegin(addr.get());
  uint64_t end = getEnd(symbols, it);

  //start disassembling instructions
  //find addres of .text
  auto dotText = it->getSection();
  uint64_t addrDotText;
  assert(bool(dotText) && "function has no section (?!?)");
  addrDotText = dotText.get()->getAddress();

  uint64_t size;
  llvm::MCInst inst;
  for(uint64_t index = 0; index < (end - addr.get()); index += size){
    auto instr_addr = addr.get() + index;
    auto b = bytes.slice((addr.get() - addrDotText) + index);
    auto status = DisAsm.getInstruction(inst, size, b, instr_addr, llvm::errs());
    if(status == llvm::MCDisassembler::DecodeStatus::Success){
      std::string s;
      raw_string_ostream rso(s);
      ip.printInst(&inst, instr_addr, "", sti, rso);
      //check out llvm::raw_string_ostream
      rso.str();
      res.addInst(ObjInstr(instr_addr, size, inst, s));
    }
    else{
      llvm::errs() << "cannot disasseble instr at " << instr_addr << "\n";
    }
  }

  return res;	
}

inline std::vector<llvm::object::SymbolRef> getTextSymbols(const llvm::object::ObjectFile& obj){
  //recover the symbols from .text
	std::vector<llvm::object::SymbolRef> textSymbols;
	for (auto symbol : obj.symbols())
	  {
	    auto sect = symbol.getSection();
	    if(bool(sect)){
	      auto name = sect.get()->getName();
	      if (isDotText(*(sect.get()))){
		textSymbols.push_back(symbol);
	      }
	    }
	  }
	
	//sort the .text's symbols by address
	std::sort(
		  textSymbols.begin(), textSymbols.end(),
		  [](llvm::object::SymbolRef& s1, llvm::object::SymbolRef& s2) {
		    auto addr1 = s1.getAddress();
		    auto addr2 = s2.getAddress();
		    assert(bool(addr1) && "symbol with no address while comp");
		    assert(bool(addr2) && "symbol with no address while comp");
		    return addr1.get() < addr2.get();

		  });
	return textSymbols;
}

inline std::vector<std::string> getFileNames(const llvm::DWARFDebugLine::LineTable* table){
  std::vector<std::string> res;
  auto kind = DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath;
  for(unsigned long i = 1; ; i++){
    if(table->hasFileAtIndex(i)){
      std::string path;
      table->getFileNameByIndex(i, "", kind, path);
      res.push_back(path);
    }
    else{
      break;
    }
  }
  return res;
}
    

inline AddrLines getAddrLines(llvm::DWARFContext& DCtx){
	AddrLines addrlines;

	auto units = DCtx.compile_units();
	

	for (auto& unit : units){ // TODO: deal with multiple units
	  const llvm::DWARFDebugLine::LineTable* table =
	    DCtx.getLineTableForUnit(unit.get());
	  if(table){
	    auto files = getFileNames(table);
	    for (auto row : table->Rows){
	      addrlines[row.Address.Address] = row.Line;
	    }
	  }
	  else
	    llvm::errs() << "table is null\n";
	}
	return addrlines;
}

inline LinesAddr getLinesAddr(const AddrLines& addrs){
  LinesAddr res;
  for(auto el : addrs){
    res[el.second].push_back(el.first);
  }
  return res;
}


inline LinesAddr getMap(const std::string& objPath, const Module& m){
  //a lot of boilerplate to instantiate all the objects needed for the disassembly
  //taken from llvm-objdump
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
  
  auto DCtx_ptr = DWARFContext::create(*Obj);
  DWARFContext* DCtx = DCtx_ptr.get();
  AddrLines addrs  = getAddrLines(*DCtx);
	
  //Creates an MCContext given an object file.
  std::string Error;
	
  auto theTriple = Obj->makeTriple();
  InitializeAllTargetInfos();
  InitializeAllTargetMCs();
  InitializeAllDisassemblers();
  const Target* theTarget =
    TargetRegistry::lookupTarget(theTriple.getTriple(), Error);
  std::unique_ptr<const MCRegisterInfo> MRI(
					    theTarget->createMCRegInfo(theTriple.getTriple()));
  assert(MRI && "no MRI");

  MCTargetOptions MCOptions;
  std::unique_ptr<const MCAsmInfo> AsmInfo(
					   theTarget->createMCAsmInfo(*MRI, theTriple.getTriple(), MCOptions));
  assert(AsmInfo && "no AsmInfo");
  MCObjectFileInfo MOFI;
  MCContext Ctx(AsmInfo.get(), MRI.get(), &MOFI);
	
  // MCDisassembler
  std::unique_ptr<const MCSubtargetInfo> STI(
					     theTarget->createMCSubtargetInfo(theTriple.getTriple(), "", ""));
  assert(STI && "no STI");
  std::unique_ptr<MCDisassembler> DisAsm(
					 theTarget->createMCDisassembler(*STI, Ctx));
  assert(DisAsm && "no DisAsm");
  std::unique_ptr<const MCInstrInfo> MII(theTarget->createMCInstrInfo());
  assert(MII && "no mcinstrinfo!");
  
  std::unique_ptr<MCInstPrinter> ip(
				    theTarget->createMCInstPrinter(
								   Triple(theTriple.getTriple()),
								   AsmInfo->getAssemblerDialect(),
								   *AsmInfo,
								   *MII,
								   *MRI));
  assert(ip && "no instr printer!");
  //end of the boilerplate code

  //retrieve the .text section
  auto dotText = getDotText(*Obj);
  auto contents = dotText.getContents();
  ArrayRef<uint8_t> bytes;
  if(bool(contents)){
    bytes = arrayRefFromStringRef(contents.get());
  }
	
  //recover the symbols from .text
  std::vector<SymbolRef> textSymbols = getTextSymbols(*Obj);
        
  //create an ObjFunction for each function in the executable
  ObjModule objM;
  for(auto& name : funcNames(m)){
    objM.addFunction(getFun(textSymbols,
		       name,
		       *DisAsm,
		       *ip,
		       *STI,		      
		       bytes));
  }
		
  //complete the mapping
  objM.setDebugLocations(addrs);
  //objM.dump();
  objM.fixPrologues();
  objM.completeDebugLoc();
  objM.dump();

  //return the line->addresses mapping
  return getLinesAddr(objM.getMap());
}
