#include <Random/Random.hpp>
#include <iostream>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  std::cout
    << "random numbers are pretty cool "
    << dl::random::GetUniform<double>() << ", "
    << dl::random::GetUniform<float>() << ", "
    << dl::random::GetUniform<long double>() << ", "
    << dl::random::GetUniform<unsigned>() << ", "
    << dl::random::GetUniform<long>() << ", "
    << dl::random::GetUniform<int>() << ", "
    << +dl::random::GetUniform<uint8_t>() << ", "
    << std::endl;
}

