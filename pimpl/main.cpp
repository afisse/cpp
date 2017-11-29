#include "book.h"

int main () {
  Book *b = new Book();
  b->print();
  delete b;
  return 0;
}
