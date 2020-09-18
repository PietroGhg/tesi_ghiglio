unsigned long fact(int n){ //72 llvm instr
  if(n <= 1) return 1; //40 llvm instr
  unsigned long a = fact(n-1); //50 llvm instr
  unsigned    long b = n * a; //60 llvm instr
  return n*a; 	    //60 llvm instr
} //24 llvm instr  
 
int main(){
  fact(9); //238 llvm instr
  fact(3); //70 llvm instr
} //1 llvm instr
