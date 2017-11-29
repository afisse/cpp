#include <string>
#include <iostream>
#include <stdexcept>

class MyException {
public:
  virtual void msg() const { std::cout << "MyException" << std::endl; }
};

class MyDirtyException : public MyException {
public:
  void msg() const { std::cout << "MyDirtyException" << std::endl; } // test without const
};

template <typename U>
const std::string& throwConvertedMongoException(const std::string& x, const U& ex)
{
  if (x.empty()){
    ex.msg();
  }
  return x;
}

template <typename F>
void bar(F f)
{
  MyDirtyException ex;
  std::cout << "bar=" << f("", ex) << std::endl;
}

const std::string& foo(const std::string& x, const MyException& ex)
{
  if (x.empty()){
    ex.msg();
  }
  return x;
}

int main(){
  const std::string& (*g) (const std::string&, const MyException&) = throwConvertedMongoException<MyException>;
  bar(g);
  bar(foo);
  return 0;
}
