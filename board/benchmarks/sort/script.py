import random

dim = 100
s = """ #pragma once
#define DIM """ + str(dim) + "\n int v[" + str(dim) + "]"

print(s+"{", end = "")
for i in range(dim-1):
    n = random.randint(1,10000)
    print(n, end = ", ")
print(random.randint(1,10000), "};")
