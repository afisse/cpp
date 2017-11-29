#include <string>
#include <iostream>
#include <stdexcept>

class MyException : public std::exception {
public:
  virtual const char* what() const throw() { return "MyException"; }
  virtual void msg() const { std::cout << "MyException" << std::endl; }
};

class MyDirtyException : public MyException {
public:
  virtual const char* what() const throw() { return "MyDirtyException"; }
  virtual void msg() const { std::cout << "MyDirtyException" << std::endl; } // test without const
};

class MyUglyException : public MyException {
public:
  virtual const char* what() const throw() { return "MyUglyException"; }
  virtual void msg() const { std::cout << "MyUglyException" << std::endl; } // test without const
};

template <typename U>
const std::string& throwConvertedMongoException(const std::string& x, U& ex)
{
  if (x.empty()){
    ex.msg();
    if (MyDirtyException* ex2 = dynamic_cast<MyDirtyException*>(&ex)) {
      throw *ex2;
    } else if (MyUglyException* ex2 = dynamic_cast<MyUglyException*>(&ex)) {
      throw *ex2;
    } else {
      throw ex;
    }
  }
  return x;
}

template <typename F>
void bar(F f)
{
  MyUglyException ex;
  std::cout << "bar=" << f("", ex) << std::endl;
}

int main(){
  bar(throwConvertedMongoException<MyException>);
  return 0;
}
