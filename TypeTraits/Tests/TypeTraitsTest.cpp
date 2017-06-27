#include <TypeTraits/TypeTraits.hpp>

#include <iostream>

int main()
{
  std::cout <<
    std::boolalpha <<
    dl::ContainsType<int>(std::tuple<double, char, int, unsigned>{})
    << std::endl;

  std::cout <<
    std::boolalpha <<
    dl::ContainsType<uint8_t>(std::tuple<double, char, int, unsigned>{})
    << std::endl;
}
