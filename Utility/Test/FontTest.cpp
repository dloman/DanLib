#include <Font/Font.hpp>

#include <iostream>

int main()
{
  auto& FontMap = dl::font::GetFontMap();

  for (const auto& Row : FontMap["a"])
  {
    for (const auto& ColumnValue : Row)
    {
      std::cout << ColumnValue << std::endl;
    }
  }
  return 0;
}

