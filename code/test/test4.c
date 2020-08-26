unsigned long fact(int n){
  if(n <= 1) return 1;
  unsigned long a = fact(n-1);
  unsigned long b = n * a;
  return a*b;
}

int main(){
  int n = 10;
  while(n > 0){
    fact(n);
    n--;
  }
}
