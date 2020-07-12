#include <stdio.h>
#include <stdlib.h>

int f(int a, int b){
  if(a > b){
        a = a + b;
  }
  else{
    a = a * b;
  }
  return a;
}

int main(){
    printf("%d", f(rand(), rand()));
}
