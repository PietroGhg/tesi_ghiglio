unsigned long fact(int n){ //48 assembly instr
  if(n <= 1) return 1; //28 assembly instr
  unsigned long a = fact(n-1); //50 assembly instr
  unsigned long b = n * a; //30 assembly instr
  return n*a; //30 assembly instr
} //60 assembly instr
 
int main(){
  fact(9); //194 assembly instr
  fact(3); //62 assembly instr
} //5 assembly instr
