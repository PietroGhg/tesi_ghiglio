//here, with optimizations, line 4 and 10 are merged into a single one
//line location in debug info is line 0 (function declaration)
int f(int a, int b){
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
  int a = f(10, 20);
  int b = f(11, 12);
}
