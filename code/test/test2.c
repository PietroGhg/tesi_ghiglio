#include <stdio.h>
#include <stdlib.h>

int g(int n){
  return n*n;
}

int f(int a, int b){
  int c = g(a);
  int d = g(b);
  return c + d;
}

int main(){
  f(10,20);
  f(30,40);
}
