#include <Serial/Serial.hpp>

#include <csignal>
#include <iostream>

std::unique_ptr<dl::serial::Serial> pSerial(nullptr);
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int main()
{
  for (const auto& SerialPort : dl::serial::Serial::GetAvailableSerialPorts())
  {
    try
    {
      auto pSerial = std::make_unique<dl::serial::Serial>(SerialPort, 115200);

      std::cout << "\nconnected to " << SerialPort << std::endl;
      pSerial->GetOnRxSignal().Connect(
        [] (const auto& Bytes) {std::cout << "rx = " << Bytes << std::endl;});

      std::string Input;
      while(std::cin >> Input)
      {
        pSerial->Write('!' + Input + '*');
      }
    }
    catch (std::exception& Exception)
    {
      std::cerr << "ERROR: " << Exception.what() << std::endl;
    }
  }
}
