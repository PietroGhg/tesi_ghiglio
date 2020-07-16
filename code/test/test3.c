int f(int n){
  int i;
  int ris = 0;
  g(10); //100
  for(i = 0; i < n; i++){
    ris += i;
  }
  return ris;
}

int main(){
  int a = f(100); 
  int b = f(10);
}
