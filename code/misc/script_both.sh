name=test3
extension=c
compiler=clang
$compiler -S -emit-llvm $name.$extension -g -o $name.ll
opt -S -load=./AddIRDebug.so -addIRdbg $name.ll > replaced.ll
$compiler replaced.ll -g -o replaced

#clang -S -emit-llvm $name.c -g
#opt -load=./Instrument/Instrument.so -legacy-inject-func-call $name.ll -S  > instr.ll
opt -load=./SimpleInstrument/SimpleInstrument.so -legacy-simple-inject test3.ll -S > instr.ll
$compiler instr.ll -o instrumented 
./instrumented > trace
./parsetrace -m=$name.ll -t=trace -b=replaced -llvm -ass -callsites 
