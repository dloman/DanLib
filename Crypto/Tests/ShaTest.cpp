#include <Crypto/Sha.hpp>
#include <iostream>
#include <iomanip>

using namespace std::literals;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  constexpr auto Sha = dl::crypto::sha256("abc");

  const uint8_t* m = reinterpret_cast<const uint8_t*>(Sha.h);

  std::stringstream Output;

  Output << std::hex << std::setfill('0');
  for (size_t i = 0; i < sizeof(Sha.h); ++i)
  {
    Output << std::setw(2) << +m[i];
    std::cout << +m[i] << std::endl;
  }

  if (
    "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"s == Output.str())
  {
    std::cout << Output.str() << std::endl;
    return 0;
  }
  else
  {
    std::cerr <<
      "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad !=" <<
      Output.str();

    return 1;
  }
}
