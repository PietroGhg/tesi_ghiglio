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

#include <bits/stdint-uintn.h>
#include <iostream>
#include "utils.hpp"
#include "replaceDebug.hpp"

// targets for disassembly
#include "llvm/Support/TargetSelect.h"

using namespace llvm;
using namespace object;


//arg 1 -> object
int main(int argc, char* argv[]){
  
  StringRef Filename(argv[1]);
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
        

  auto mainF = getFun(textSymbols, "main", *DisAsm, bytes);
  auto fF = getFun(textSymbols, "f", *DisAsm, bytes);
  sourceLoc lastLoc;
  std::vector<ObjFunction> v{mainF, fF};
  completeMapping(addrs, v);
  for(auto& i : mainF.getInstructions()){
    if(addrs.find(i.getAddr()) != addrs.end()){
      lastLoc = addrs[i.getAddr()];	    
    }
    i.setDebugLoc(lastLoc);
    i.dump();
    std::cout << "\n";
  }
  
  for(auto el : addrs){
    std::cout << std::hex << el.first << ": ";
    std::cout << std::dec << el.second << "\n";
  }

  for(auto el : getLinesAddr(addrs)){
    std::cout << el.first << ": ";
    for(auto addr : el.second){
      std::cout << std::hex << addr << " ";
    }
    std::cout << "\n";
  }
}
