name=test5
clang -S -emit-llvm -g $name.c
opt -load=./Instrument.so -legacy-inject-func-call $name.ll -S  > new.ll
clang new.ll
./a.out > trace
./parsetrace $name.ll trace $name.c
