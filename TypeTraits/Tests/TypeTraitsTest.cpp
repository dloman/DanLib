#include <TypeTraits/TypeTraits.hpp>

#include <iostream>

int main()
{
  std::cout <<
    std::boolalpha <<
    dl::HasType<int, std::tuple<double, char, int, unsigned>>::value
    << std::endl;

  std::cout <<
    std::boolalpha <<
    dl::HasType<uint8_t, std::tuple<double, char, int, unsigned>>::value
    << std::endl;
}
