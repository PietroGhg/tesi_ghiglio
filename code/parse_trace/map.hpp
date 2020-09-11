#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/StringExtras.h"	//arrayRefFromStringRef
#include "llvm/ADT/StringRef.h"
#include "llvm/DebugInfo/DIContext.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCTargetOptions.h"
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
  sourceLoc debugLoc = 0; // 0 -> no debugLoc
public:
  ObjInstr(uint64_t _addr, uint64_t _size, llvm::MCInst _inst):
    addr(_addr), size(_size), inst(_inst){}
  uint64_t getAddr() const { return addr; }
  uint64_t getSize() { return size; }
  llvm::MCInst getInst() { return std::move(inst); }
  void setDebugLoc(sourceLoc _debugLoc) { debugLoc = _debugLoc; }
  sourceLoc getDebugLoc() const { return debugLoc; }

  void dump(){
    std::cout << std::hex << "addr: " << addr << " ";
    if(debugLoc){
      std::cout << std::dec << "debug: " << debugLoc <<  " ";
    }
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
  std::string getName() { return name; }
  uint64_t getBegin() { return begin; }
  uint64_t getEnd() { return (instructions.end() - 1)->getAddr(); }
  void setBegin(uint64_t _begin){ begin = _begin; }
  void addInst(ObjInstr inst){ instructions.push_back(inst); }
  const std::vector<ObjInstr>& getInstructions() const {
    return instructions;
  }
  std::vector<ObjInstr>& getInstructions(){
    return instructions;
  }
    
  void dump(){
    std::cout << "name: " << name << "\n";
    std::cout << "begin: " << std::hex << begin
	      << " end: " << std::hex << getEnd() << "\n";
    for(auto i : instructions){
      i.dump();
    }
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
	    res.addInst(ObjInstr(instr_addr, size, inst));
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

inline void completeMapping(AddrLines& addrs,
			    const std::vector<ObjFunction>& funcs){
  sourceLoc lastLoc;
  for(auto& f : funcs){
    for(auto& i : f.getInstructions()){
      if(addrs.find(i.getAddr()) != addrs.end()){
	lastLoc = addrs[i.getAddr()];
      }
      else{
	addrs[i.getAddr()] = lastLoc;
      }
    }
  }
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
	
  // MCInstrAnalysis
  std::unique_ptr<const MCInstrInfo> MII(theTarget->createMCInstrInfo());
  std::unique_ptr<const MCInstrAnalysis> MIA(
					     theTarget->createMCInstrAnalysis(MII.get()));

  auto dotText = getDotText(*Obj);
  auto contents = dotText.getContents();
  ArrayRef<uint8_t> bytes;
  if(bool(contents)){
    bytes = arrayRefFromStringRef(contents.get());
  }
	
  //recover the symbols from .text
  std::vector<SymbolRef> textSymbols = getTextSymbols(*Obj);
        
  //create an ObjFunction for each function in the executable
  std::vector<ObjFunction> v;
  for(auto& name : funcNames(m)){
    v.push_back(getFun(textSymbols, name, *DisAsm, bytes));
  }
		
  //complete the mapping
  completeMapping(addrs, v);

  //return the line->addresses mapping
  return std::move(getLinesAddr(addrs));
}
