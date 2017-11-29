#ifndef cursor_impl_h
#define cursor_impl_h

#include "mongo_cursor.hpp"

class Cursor;
class CursorImpl {
  friend class Cursor;
private:
  CursorImpl ();
  CursorImpl (const CursorImpl&);
};

#endif
