#ifndef BOOK_IMPL_H
#define BOOK_IMPL_H

#include "book.h"
#include <iostream>
#include <string>

class Book::BookImpl
{
 public:
  void print();
 private:
  std::string  m_Contents;
  std::string  m_Title;
};

#endif
