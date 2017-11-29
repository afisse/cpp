#include <iostream>

enum day
{
  Monday, Tuesday, Wednesdau, Thursday, Friday, Saturday, Sunday
};

day op1 (day d) {
  return (day)((d+1)%7);
}

day* op2 (day* d) {
  *d = (day) ((*d + 1) % 7);
  return d;
}

day& op3 (day& d) {
  d = (day) ((d + 1) % 7);
  return d;
}

int main() {
  day d(Sunday);
  std::cout << op1(d) << std::endl;
  std::cout << *op2(&d) << std::endl;
  std::cout << op3(d) << std::endl;
  return 0;
}
