#include<map.h>

std::vector<std::string> funcNames(const llvm::Module& m){
  std::vector<std::string> res;
  for(auto& f : m){
    if(!f.isDeclaration())
      res.push_back(f.getName().str());
  }
  return res;
}

bool isDotText(const llvm::object::SectionRef section){
  auto name = section.getName();
  return bool(name) && name.get() == ".text";
}

llvm::object::SectionRef getDotText(const llvm::object::ObjectFile& obj){
  for(auto sec : obj.sections()){
    if(isDotText(sec)){
      return std::move(sec);
    }
  }
  assert(false && "No .text");
}

uint64_t getEnd(const std::vector<llvm::object::SymbolRef>& symbols,
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

std::pair<ObjFunction,bool> getFun(const std::vector<llvm::object::SymbolRef>& symbols,
					  const std::string& name,
					  llvm::MCInstPrinter& ip,
					  const llvm::MCDisassembler& DisAsm,
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
  if(it == symbols.end()){
    //errs() << "Cannot find: " << name << "\n\n";
    //assert(it != symbols.end() && "symbol name not found");
    return std::pair<ObjFunction, bool>(res, false);
  }
  
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

  for(uint64_t index = 0; index < (end - addr.get()); index += size){
    llvm::MCInst inst;
    auto instr_addr = addr.get() + index;
    auto b = bytes.slice((addr.get() - addrDotText) + index);
    auto status = DisAsm.getInstruction(inst, size, b, instr_addr, llvm::errs());
    if(status == llvm::MCDisassembler::DecodeStatus::Success){
      std::string s;
      raw_string_ostream rso(s);
      ip.printInst(&inst, instr_addr, "", sti, rso);
      rso.str();
      res.addInst(ObjInstr(instr_addr, size, inst, s));
    }
    else{
      llvm::errs() << "cannot disasseble instr at ";
      errs().write_hex(instr_addr) << "\n";
      //TODO: this works only sometimes
      size = 4;
      std::string s("dummy");
      res.addInst(ObjInstr(instr_addr, size, inst, s));
    }
  }

  return std::pair<ObjFunction, bool>(res,true);	
}

std::vector<llvm::object::SymbolRef> getTextSymbols(const llvm::object::ObjectFile& obj){
  //recover the symbols from .text
	std::vector<llvm::object::SymbolRef> textSymbols;
	for (auto symbol : obj.symbols())
	  {
	    auto sect = symbol.getSection();
	    if(bool(sect)){
	      //auto name = sect.get()->getName();
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


AddrLines getAddrLines(llvm::DWARFContext& DCtx){
	AddrLines addrlines;

	auto units = DCtx.compile_units();
	

	for (auto& unit : units){ 
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
	return addrlines;
}

LinesInstr getLinesAddr(const InstrLines& addrs){
  LinesInstr res;
  for(auto el : addrs){
    res[el.second].push_back(el.first);
  }
  return res;
}

ObjModule getObjM(const std::string& objPath,
		  const std::string& MCPU,
		  const Module& m,
		  const bool printDisAss){
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
	
  //Retrieve features
  SubtargetFeatures Features = Obj->getFeatures();
  std::unique_ptr<const MCSubtargetInfo> STI(
					     theTarget->createMCSubtargetInfo(theTriple.getTriple(), MCPU, Features.getString()));
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
    auto f = getFun(textSymbols,
		    name,
		    *ip,
		    *DisAsm,
		    *STI,		      
		    bytes);
    if(f.second)
      objM.addFunction(f.first);
  }
		
  //complete the mapping
  objM.setDebugLocations(addrs);
  //objM.dump();
  objM.fixPrologues();
  objM.completeDebugLoc();
  if(printDisAss)
    objM.dump();

  return objM;
}

LinesInstr getMap(const std::string& objPath,
		  const std::string& MCPU,
		  const Module& m,
		  const bool printDisAss){

  auto objM = getObjM(objPath, MCPU, m, printDisAss);
  return getLinesAddr(objM.getMap());
}
