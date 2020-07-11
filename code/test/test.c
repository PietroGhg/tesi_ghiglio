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