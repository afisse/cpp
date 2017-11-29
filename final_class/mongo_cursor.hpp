#ifndef mongo_cursor_h
#define mongo_cursor_h

class MongoCursor {
public:
  MongoCursor();
  int count();
  bool compare (MongoCursor c);
private:
  static int _id_count;
  int _id;
};

#endif
