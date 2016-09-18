#include <Signal/Signal.hpp>
#include <iostream>


int main()
{
  auto Lambda =
    [] (auto Value) { std::cout << "Value = " << Value << std::endl; };

  dl::Signal<double> Double;

  Double.Connect(Lambda);

  Double(1337.420);
  Double(420.69);

  dl::Signal<std::string> String;

  String.Connect(Lambda);

  String("fuck");
  String("yeah!!!");
}



