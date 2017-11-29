#include "mongo_cursor.hpp"

int MongoCursor::_id_count = 0;

MongoCursor::MongoCursor():_id(this->_id_count){
  ++(this->_id_count);
}

int MongoCursor::count(){
  return this->_id_count;
}
bool MongoCursor::compare (MongoCursor c) {
  return this->_id == c._id;
}
