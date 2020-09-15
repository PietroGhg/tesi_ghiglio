#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/StringExtras.h"	//arrayRefFromStringRef
#include "llvm/ADT/StringRef.h"
#include "llvm/DebugInfo/DIContext.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
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

using LinesAddr = std::map<uint64_t, std::vector<uint64_t>>;
using AddrLines = std::map<uint64_t, uint64_t>;
using sourceLoc = unsigned long;

class ObjInstr{
private:
  uint64_t addr;
  uint64_t size;
  llvm::MCInst inst;
  std::string disass;
  sourceLoc debugLoc = 0; // 0 -> no debugLoc
public:
  ObjInstr(uint64_t _addr, uint64_t _size,
	   llvm::MCInst _inst,
	   std::string& _disass):
    addr(_addr), size(_size), inst(_inst), disass(_disass){}
  uint64_t getAddr() const { return addr; }
  uint64_t getSize() { return size; }
  void setDebugLoc(sourceLoc _debugLoc) { debugLoc = _debugLoc; }
  bool hasDebugLoc(){ return debugLoc != 0; }
  sourceLoc getDebugLoc() const { return debugLoc; }

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

  void fixPrologue(){
    bool found_once = false;
    sourceLoc loc;
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
    return std::move(res);
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
  return std::move(res);
}

inline llvm::object::SectionRef getDotText(const llvm::object::ObjectFile& obj){
  for(auto sec : obj.sections()){
    if(isDotText(sec)){
      return std::move(sec);
    }
  }
  llvm::errs() << "no .text!\n";
  throw;
}
  

inline ObjFunction getFun(std::vector<llvm::object::SymbolRef> symbols,
			  std::string name,
			  const llvm::MCDisassembler& DisAsm,
			  llvm::MCInstPrinter& ip,
			  const llvm::MCSubtargetInfo& sti,
			  const llvm::ArrayRef<uint8_t>& bytes){
  ObjFunction res(name);
  for(auto it = symbols.begin(); it != symbols.end(); it++){
    auto symName = it->getName();
    if(bool(symName) && symName.get() == name){
      auto addr = it->getAddress();
      if(bool(addr)){
	//compute end of function: either next symbol or next section
	res.setBegin(addr.get());
	uint64_t end;
	if(std::next(it) != symbols.end()){
	  llvm::object::SymbolRef nextSym = *std::next(it);
	  auto nextAddr = nextSym.getAddress();
	  if(bool(nextAddr)){
	    end = nextAddr.get();
	  }
	  else{
	    llvm::errs() << "no address!\n";
	    throw;
	  }
	}
	else{
	  auto section = it->getSection();
	  if(bool(section)){
	    end = section.get()->getAddress() + section.get()->getSize();
	  }
	  else{
	    llvm::errs() << "no section!\n";
	    throw;
	  }
	}

	//start disassembling instructions
	//find addres of .text
	auto dotText = it->getSection();
	uint64_t addrDotText;
	if(bool(dotText)){
	  addrDotText = dotText.get()->getAddress();
	}
	else{
	  llvm::errs() << "function has no section ?!?\n";
	  throw;
	}
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
      }
      else{
	llvm::errs() << "no address!\n";
	throw;
      }
      break;
    }
  }

  return std::move(res);	
}

inline std::vector<llvm::object::SymbolRef> getTextSymbols(const llvm::object::ObjectFile& obj){
  //recover the symbols from .text
	std::vector<llvm::object::SymbolRef> textSymbols;
	for (auto symbol : obj.symbols())
	  {
	    auto sect = symbol.getSection();
	    if (bool(sect)){
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
		    bool res = false;
		    if(bool(addr1) && bool(addr2)){
		      res = addr1.get() < addr2.get();
		    }
		    return res;
		  });
	return std::move(textSymbols);
}

inline AddrLines getAddrLines(llvm::DWARFContext& DCtx){
	AddrLines addrlines;

	auto units = DCtx.compile_units();
	

	for (auto& unit : units){ // TODO: deal with multiple units
	  const llvm::DWARFDebugLine::LineTable* table =
	    DCtx.getLineTableForUnit(unit.get());
	  if(table){
	    for (auto row : table->Rows){
	      addrlines[row.Address.Address] = row.Line;
	    }
	  }
	  else
	    llvm::errs() << "table is null\n";
	}
	return std::move(addrlines);
}

inline LinesAddr getLinesAddr(const AddrLines& addrs){
  LinesAddr res;
  for(auto el : addrs){
    res[el.second].push_back(el.first);
  }
  return std::move(res);
}


inline LinesAddr getMap(std::string objPath, Module& m){
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
  if (!MRI)
    errs() << "no MRI\n";
  MCTargetOptions MCOptions;
  std::unique_ptr<const MCAsmInfo> AsmInfo(
					   theTarget->createMCAsmInfo(*MRI, theTriple.getTriple(), MCOptions));
  if (!AsmInfo)
    errs() << "no AsmInfo\n";
  MCObjectFileInfo MOFI;
  MCContext Ctx(AsmInfo.get(), MRI.get(), &MOFI);
	
  // MCDisassembler
  std::unique_ptr<const MCSubtargetInfo> STI(
					     theTarget->createMCSubtargetInfo(theTriple.getTriple(), "", ""));
  if (!STI)
    errs() << "no STI\n";
  std::unique_ptr<MCDisassembler> DisAsm(
					 theTarget->createMCDisassembler(*STI, Ctx));
  if (!DisAsm)
    errs() << "no DisAsm\n";
  std::unique_ptr<const MCInstrInfo> MII(theTarget->createMCInstrInfo());
  if(!MII)
    errs() << "no mcinstrinfo!\n";
  
  std::unique_ptr<MCInstPrinter> ip(
				    theTarget->createMCInstPrinter(
								   Triple(theTriple.getTriple()),
								   AsmInfo->getAssemblerDialect(),
								   *AsmInfo,
								   *MII,
								   *MRI));
  if(!ip)
    errs() << "no instr printer!\n";
  
								   
	
  // MCInstrAnalysis
  

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
  objM.dump();
  objM.fixPrologues();
  objM.dump();

  //return the line->addresses mapping
  return std::move(getLinesAddr(objM.getMap()));
}
