#include <Signal/Signal.hpp>
#include <iostream>

int main()
{
  auto Lambda =
    [] (auto Value) { std::cout << "Value = " << Value << std::endl; };

  dl::Signal<double> Double;

  Double.Connect(Lambda);
  Double.Connect(Lambda);

  Double(1337.420);
  Double(420.69);

  dl::Signal<std::string> String;

  String.Connect(Lambda);

  String("fuck");
  String("yeah!!!");


  dl::Signal<double, std::string> DoubleString;

  DoubleString.Connect(
    [] (double Double, std::string String)
    {
      std::cout << "Double = " << Double << " String = " << String << std::endl;
    });

  DoubleString(69.420, std::string("fuck"));
  DoubleString(1337.69, std::string("yeah"));

  dl::Signal<void> Null;

  Null.Connect([] {std::cout << "Null Signaled" << std::endl;});
  Null();
  Null();
}



