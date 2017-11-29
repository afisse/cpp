#include <string>
#include <iostream>
#include <stdexcept>

template <typename U>
const std::string& throwConvertedMongoException(const std::string& x, const U& ex)
{
  if (x.empty()){
    throw ex;
  }
  return x;
}

class MyEx : public std::exception
{
public:
virtual const char* what() const throw() { return "woopwoop"; }
};

template <typename F>
void bar(F f)
{
  MyEx ex;
  std::cout << "BAAAA" << std::endl;
  std::cout << "bar=" << f("",ex) << std::endl;
}

int main(){
  const std::string& (*g) (const std::string&, const std::exception&) = throwConvertedMongoException<std::exception>;
  bar(g);
  return 0;
}
