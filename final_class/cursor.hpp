#ifndef cursor_h
#define cursor_h

#include "cursor_impl.hpp"

class Cursor : public virtual CursorImpl {
public:
  Cursor ();
  int count ();
  bool compare (Cursor c);
private:
  MongoCursor _mongo_delegate;
};

#endif
