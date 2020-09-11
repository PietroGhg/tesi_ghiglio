name=test
clang -S -emit-llvm $name.c -g
opt -S -strip-debug-declare $name.ll > stripped.ll
opt -S -load=./AddIRDebug.so -addIRdbg stripped.ll > new.ll
clang new.ll -g -o new
./map new new.ll
