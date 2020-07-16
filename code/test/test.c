#include <stdlib.h>
#include <stdio.h>
//here, with optimizations, line 4 and 10 are merged into a single one
//line location in debug info is line 0 (function declaration)
int f(int a, int b){
  printf("a\n");
    if(a > b){
        a = a + b;
    }
    else{
        while(a <= b){
            a++;
        }
        a = a + b;
    }
    return a;
}

int main(){
    int a = f(rand(), rand());
    printf("%d", a);
}
