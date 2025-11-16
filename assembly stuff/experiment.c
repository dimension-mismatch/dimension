int three_x_plus_one(int x){
  return 3 * x + 1;
}

int main(){
  int a = 5;
  int b = 6;

  int c = (a + b) + (a + three_x_plus_one(b));
}