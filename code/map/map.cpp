#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/Object/Binary.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/DebugInfo/DIContext.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/ADT/StringExtras.h" //arrayRefFromStringRef

//targets for disassembly
#include "llvm/Support/TargetSelect.h"

#include <string>

using namespace llvm;
using namespace object;
using LinesMat = std::vector<std::vector<SectionedAddress>>;


LinesMat getLinesMat(DWARFContext& DCtx){
    LinesMat lines;
    
    auto units = DCtx.compile_units();
    
    for(auto& unit : units){ //TODO: deal with multiple units
        const DWARFDebugLine::LineTable* table = DCtx.getLineTableForUnit(unit.get());
        if(table){
            int max = 0;
            for(auto row : table->Rows){
                if(row.Line > max) max = row.Line;
            }
            //matrix line x addresses
            lines.resize(max+1);
            for(auto row : table->Rows){
                lines[row.Line].push_back(row.Address);
            }
            
        }
        else
            errs() << "table is null\n";
    }
    return lines;
}

int main(int argc, char* argv[]){
    
    StringRef Filename(argv[1]);
    ErrorOr<std::unique_ptr<MemoryBuffer>> BuffOrErr = MemoryBuffer::getFileOrSTDIN(Filename);      
    std::unique_ptr<MemoryBuffer> Buffer = std::move(BuffOrErr.get());  
    Expected<std::unique_ptr<Binary>> BinOrErr = object::createBinary(*Buffer);  
    auto Obj = dyn_cast<ObjectFile>(BinOrErr->get()); 
    auto DCtx_ptr = DWARFContext::create(*Obj);
    DWARFContext* DCtx = DCtx_ptr.get();

    LinesMat lines = getLinesMat(*DCtx);
    int i = 0;
    for(auto line : lines){
        errs() << i << ": " << line.size() << " ";
        for(auto& addr : line){
            errs() << addr.Address << " ";
        }
        errs() << "\n";
        i++;
    }

    /*
    //Creates an MCContext given an object file.
    std::string Error;
    auto theTriple = Obj->makeTriple();
    llvm::InitializeAllTargets();
    const Target* theTarget = TargetRegistry::lookupTarget(theTriple.getTriple(), Error);
    errs() << Error << "\n";
    std::unique_ptr<const MCRegisterInfo> MRI( theTarget->createMCRegInfo(theTriple.str()) );
    MCTargetOptions MCOptions;
    std::unique_ptr<const MCAsmInfo> AsmInfo( theTarget->createMCAsmInfo(*MRI, theTriple.str(), MCOptions) );
    MCObjectFileInfo MOFI;
    MCContext Ctx(AsmInfo.get(), MRI.get(), &MOFI);

    //MCDisassembler
    std::unique_ptr<const MCSubtargetInfo> STI( 
        theTarget->createMCSubtargetInfo(theTriple.str(), "", ""));
    std::unique_ptr<MCDisassembler> DisAsm(
      theTarget->createMCDisassembler(*STI, Ctx));

    //MCInstrAnalysis
    std::unique_ptr<const MCInstrInfo> MII(theTarget->createMCInstrInfo());
    std::unique_ptr<const MCInstrAnalysis> MIA(
      theTarget->createMCInstrAnalysis(MII.get()));
    
    //MCDisassembler->getInstruction
    for(auto& Section : Obj->sections()){
        if(Section.isText()){
            errs() << Section.getAddress() << "\n";
            ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(Section.getContents().get());
            

        }
    }
    
    /*MCInstrInfo info;
    uint64_t Size;
    for(line : lines){
        for(auto& addr : line){
            //DisAsm->getInstruction(info, Size, ????, addr.Address, errs());
        }
    }*/
    

    
}