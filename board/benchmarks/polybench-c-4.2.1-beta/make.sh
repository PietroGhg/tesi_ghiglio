source=linear-algebra/blas/gemver/gemver.c
dest=build/gemver
gcc -O3 -I utilities -I linear-algebra/kernels/atax utilities/polybench.c $source -o $dest
