unsigned long fact(int n){
  if(n <= 1) return 1;
  unsigned long a = fact(n-1);
  unsigned long b = n * a;
  return n*a;
}

int main(){
  fact(9);
  fact(3);
}
