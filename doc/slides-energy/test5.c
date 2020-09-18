int f(int n);
int g(int n);
int h(int n);

int main(){
  f(3);
  g(2);
  h(1);
}

int h(int n){
  if(n < 1) return 1;
  int a = h(n-1);
  int b = h(n-2);
  int c = f(a/b);
  return c;
}

int g(int n){
  int a = h(n-1);
  return a;
}

int f(int n){
  int a = g(n-1);
  return a;
}
