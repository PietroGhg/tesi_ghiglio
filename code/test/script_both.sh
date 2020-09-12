name=test
clang -S -emit-llvm $name.c -g
opt -S -load=./AddIRDebug.so -addIRdbg $name.ll > replaced.ll
clang replaced.ll -g -o replaced

#clang -S -emit-llvm $name.c -g
opt -load=./Instrument.so -legacy-inject-func-call $name.ll -S  > instr.ll
clang instr.ll -o instrumented
./instrumented > trace
./parsetrace $name.ll trace $name.c replaced
