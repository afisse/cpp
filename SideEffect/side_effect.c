#include <iostream>



int x = 12;

int foo () {
  //x = 13;
  return 0;
}

int bar () {
  //x = 14;
  return 0;
}

int fun (int a, int b) {
  return a + b;
}

int main () {
  fun (foo(), bar());
  printf("x = %d\n", x);

  std::cout << foo() << ' ' << bar() << std::endl;
  printf("x = %d\n", x);
  return 0;
}
