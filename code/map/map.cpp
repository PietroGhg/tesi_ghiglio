/*Dwarfcontext::create
getlinetableforunit

per aprire: string -> memory buffer -> object::createBinary -> 
auto *Obj = dyn_cast<ObjectFile>(BinOrErr->get()), Dwarfcontext::create(obj, ...)*/
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/Object/Binary.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/DebugInfo/DIContext.h"

using namespace llvm;
using namespace object;

int main(int argc, char* argv[]){
    StringRef Filename(argv[1]);
    ErrorOr<std::unique_ptr<MemoryBuffer>> BuffOrErr = MemoryBuffer::getFileOrSTDIN(Filename);
    
    std::unique_ptr<MemoryBuffer> Buffer = std::move(BuffOrErr.get());
    Expected<std::unique_ptr<Binary>> BinOrErr = object::createBinary(*Buffer);
    
    auto *Obj = dyn_cast<ObjectFile>(BinOrErr->get());
    std::unique_ptr<DWARFContext> DCtx_ptr = DWARFContext::create(*Obj);
    DWARFContext* DCtx = DCtx_ptr.get();
    DIDumpOptions options;
    for(auto& unit : DCtx->compile_units()){
        unit.get()->dump(errs(), options);
    }

    
}