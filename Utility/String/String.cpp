#include "String.hpp"

namespace dl
{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  std::vector<std::string> Split(std::string String, char Delimiter)
  {
    std::vector<std::string> Split;

    for (
      auto Position = String.find(Delimiter);
      Position != std::string::npos;
      Position = String.find(Delimiter))
    {
      Split.emplace_back(String.substr(0, Position));

      String = String.substr(Position + 1);
    }

    if (!String.empty())
    {
      Split.push_back(std::move(String));
    }

    return Split;
  }
}
