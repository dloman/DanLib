#include <TypeTraits/TypeTraits.hpp>
#include <boost/hana.hpp>

#include <iostream>

int main()
{

  static_assert(
    dl::ContainsType<int, std::tuple<double, char, int, unsigned>>{},
    "ContainsType didnt work");

  static_assert(
    !dl::ContainsType<float, std::tuple<double, char, int, unsigned>>{},
    "ContainsType didnt work");

  static_assert(
    dl::ContainsType<
      float,
      std::tuple<double, char, int, unsigned, boost::hana::tuple<std::string, double, float>>>{},
    "ContainsType didnt work");

  static_assert(
    dl::ContainsType<
      float,
      std::tuple<double, char, int, unsigned, std::tuple<std::string, double, float>>>{},
    "ContainsType didnt work");

  static_assert(
    !dl::ContainsType<
      float,
      std::tuple<double, char, int, unsigned, boost::hana::tuple<std::string, double>>>{},
    "ContainsType didnt work");

  std::cout <<
    std::boolalpha <<
    dl::ContainsType<int, std::tuple<double, char, int, unsigned>> {}
    << std::endl;

    std::cout <<
      std::boolalpha <<
      dl::ContainsType<uint8_t, std::tuple<double, char, int, unsigned>>{}
      << std::endl;
}
