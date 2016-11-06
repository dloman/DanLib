#include <String/String.hpp>
#include <iostream>

int main()
{
  std::string TestString = "it would be cool if\nthis were in a vector";

  std::vector<std::string> Truth {"it would be cool if", "this were in a vector"};

  if (!(dl::Split(TestString) == Truth))
  {
    std::cerr << "Fail" << std::endl;
  }

  return 0;
}
