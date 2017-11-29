#include "cursor.hpp"

Cursor::Cursor(){}

int Cursor::count () {
  return this->_mongo_delegate.count();
}

bool Cursor::compare (Cursor c) {
  return this->_mongo_delegate.compare (c._mongo_delegate);
}

