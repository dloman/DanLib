#include <Serial/Serial.hpp>
#include <iostream>

int main()
{
  dl::serial::Serial Serial("/dev/ttyACM0", 115200);

  Serial.GetOnRxSignal().Connect(
    [] (const auto& Bytes) { std::cout << "Recieved: " << Bytes << std::endl; });

  Serial.GetConnectionErrorSignal().Connect(
    [] (const auto& Error) { std::cout << "Error: " << Error << std::endl; });

  Serial.Write("G92 Y0");
  std::this_thread::sleep_for(std::chrono::seconds(10));

  while (true)
  {
    for (auto i = 0; i < 1300; i+=100)
    {
      std::string Out = "G1 Y" + std::to_string(i) + " F15000\n";
      std::cout << "writing " << Out << std::endl;

      Serial.Write(Out);

      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

  }

  return 0;
}
