#include <iostream> 

class Wheel {
  int speed;
public:
  Wheel () : speed (10) {}
  Wheel (int s) : speed (s) {}
  int getSpeed(){
    return speed;
  }
  void setSpeed(int speed){
    this->speed = speed;
  }
};

class Car{
private:
  Wheel wheel;
public:
  int getCarSpeed(){
    return wheel.getSpeed();
  }
  char const* getName(){
    return "My Car is a Super fast car";
  }
};

int main(){
  Car myCar;
  std::cout << myCar.getCarSpeed() << std::endl;
}
