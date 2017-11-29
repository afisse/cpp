/* Bottle - Class and Object Example */
#include <iostream>
#include <iomanip>
 
using namespace std;
 
class Bottle
{
private:      // variables are modified by member functions of class
  int iFill;    // dl of liquid
     
public:
  Bottle()    // Default Constructor
    : iFill(3)  // They start with 3 dl of liquid
  {
    // More constructor code would go here if needed.
  }
     
  bool sip() // return true if liquid was available
  {

    if (iFill > 0)
      {
	--iFill;
	return true;
      }
    else
      {
	return false;
      }

  }
 
  int level() const  // return level of liquid dl
  {
    return iFill;
  }
};  // Class declaration has a trailing semicolon
 
int main()
{
  // terosbottle object is an instance of class Bottle
  Bottle terosbottle;
  cout << "In the beginning, mybottle has "
       << terosbottle.level()
       << "  dl of liquid"
       << endl;
   
  while (terosbottle.sip())
    {
      cout << "Mybottle has "
	   << terosbottle.level()
	   << " dl of liquid"
	   << endl;
    }
   
  return 0;
}
