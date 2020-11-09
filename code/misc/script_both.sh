name=test_cpp/test
extension=cpp
compiler=clang++
$compiler -S -emit-llvm $name.$extension -g -o $name.ll
opt -S -load=./AddIRDebug.so -addIRdbg $name.ll > replaced.ll
$compiler replaced.ll -g -o replaced

#clang -S -emit-llvm $name.c -g
opt -load=./Instrument/Instrument.so -legacy-inject-func-call $name.ll -S  > instr.ll
$compiler instr.ll -o instrumented 
./instrumented > trace
./parsetrace $name.ll trace replaced
