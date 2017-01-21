#include <Crypto/Sha.hpp>
#include <iostream>
#include <iomanip>

using namespace std::literals;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  constexpr auto Sha = dl::crypto::sha256("abc");

  const uint8_t* m = reinterpret_cast<const uint8_t*>(Sha.Sum.data());

  std::stringstream Output;

  static_assert(3205920954 == Sha.Sum[0], "shits fucked up");
  static_assert(3939434895 == Sha.Sum[1], "shits fucked up");
  static_assert(3728752961 == Sha.Sum[2], "shits fucked up");
  static_assert(589475421 == Sha.Sum[3], "shits fucked up");
  static_assert(2741044144 == Sha.Sum[4], "shits fucked up");
  static_assert(2625247126 == Sha.Sum[5], "shits fucked up");
  static_assert(1644105908 == Sha.Sum[6], "shits fucked up");
  static_assert(2903834866 == Sha.Sum[7], "shits fucked up");

  Output << std::hex << std::setfill('0');
  for (size_t i = 0; i < sizeof(Sha.Sum); ++i)
  {
    Output << std::setw(2) << +m[i];
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
