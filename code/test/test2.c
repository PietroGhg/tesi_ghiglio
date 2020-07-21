#include <stdio.h>
#include <stdlib.h>

int g(int n){ //basic block 0
  return n*n;
}

int f(int a, int b){ //basic block 1
  int c = g(a); //line 9
  int d = g(b); //line 10
  return c + d;
}

int main(){ //basic block 2
  f(10,20); //line 15
  f(30,40); //line 16
}
