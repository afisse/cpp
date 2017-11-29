#ifndef BOOK_H
#define BOOK_H

class Book
{
 public:
  Book();
  ~Book();
  void print();
 private:
  class BookImpl;
  BookImpl* m_p;
};

#endif
