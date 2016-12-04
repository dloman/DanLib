#include <Math/Vector.hpp>

#include <iostream>

int main()
{
  dl::math::Vector<int, 4> Vec1({1,2,3,4});

  dl::math::Vector<int, 4> Vec2({4,3,2,1});

  auto Sum = Vec1 + Vec2;

  dl::math::Vector<int, 4> SumOfSum = Sum + Vec1 + Sum + Vec2;

  for (auto i = 0u; i < 4; ++i)
  {
    std::cout << Sum[i] << ',';
  }

  std::cout << std::endl;

  for (auto i = 0u; i < 4; ++i)
  {
    std::cout << SumOfSum[i] << ',';
  }

  std::cout << std::endl;

  for (const auto& Value : SumOfSum)
  {
    std::cout << Value << ',';
  }

  std::cout << std::endl;

  return 0;
}

