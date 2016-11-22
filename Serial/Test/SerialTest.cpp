#include <Serial/Serial.hpp>

#include <csignal>
#include <iostream>

std::unique_ptr<dl::serial::Serial> pSerial(nullptr);
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int main()
{
  std::cout << "Possible Serial Ports:" << std::endl;
  for (const auto& SerialPort : dl::serial::Serial::GetAvailableSerialPorts())
  {
    std::cout << "- " << SerialPort << std::endl;
  }
  std::cout << "\nOpening /dev/ttyACM0" << std::endl;

  try
  {
    auto pSerial = std::make_unique<dl::serial::Serial>("/dev/ttyACM0", 115200);

    pSerial->GetOnRxSignal().Connect(
      [] (const auto& Bytes) {std::cout << Bytes << std::endl;});

    std::this_thread::sleep_for(std::chrono::seconds(10));

    pSerial->Write("G28 Y0\n");
    std::cout << "going home" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    pSerial->Write("G1 Y500 F15000\n");
    std::cout << "going right" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));

    pSerial->Write("G28 Y0\n");
    std::cout << "going home" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
/*
    for (auto i = 0; i < 1300; i+=100)
    {
      std::string Out = "G1 Y" + std::to_string(i) + " F15000\n";
      std::cout << "writing " << Out << std::endl;

      pSerial->Write(Out);

      std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }

    pSerial->Write("G28 Y0 F15000\n");*/
      std::this_thread::sleep_for(std::chrono::milliseconds(1500));
   std::cout << "done" << std::endl;
  }
  catch (std::exception& Exception)
  {
    std::cerr << "ERROR: Unable to connect to /dev/ttyACM0" << std::endl;
    return 1;
  }
}
