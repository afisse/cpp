#include "cursor.hpp"
#include <iostream>

using namespace std;

class MyCursor : Cursor {

};

int main (){
  Cursor c1;
  Cursor c2;
  cout << c1.count() << endl;
  cout << c1.compare (c1) << endl;
  cout << c1.count() << endl;
  cout << c1.compare (c2) << endl;
  cout << "nb copy = " << c1.count() << endl;
  return 0;
}
