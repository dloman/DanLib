
#include <Tcp/Client.hpp>
#include <iostream>
#include <array>

using namespace std;
using namespace std::literals;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, const char** argv)
{
  std::string Host = "localhost";


  int Port = 80;

  if (argc > 2)
  {
    Port = std::atoi(argv[2]);
  }
  cout << "Client connecting on localhost 8080" << endl;

  std::atomic<bool> Finished = false;

  dl::tcp::Client Client("localhost", 80);

  Client.GetOnRxSignal().Connect([] (const auto& Bytes) { cout << Bytes << endl;});

  Client.GetOnDisconnectSignal().Connect(
    [&] { cout << "server has disconnected" << endl; Finished = true;});

  Client.GetConnectionSignal().Connect([&]
    {
      std::string Hostname = Host;

      if (Port != 80)
      {
        Hostname += std::to_string(Port);
      }

      Client.Write(
        "GET / HTTP/1.1\r\nUser-Agent: DanLib/0.0.1 (linux-gnu)\r\n"s +
        "Accept: */*\r\nAccept-Encoding: identity\r\nHost: " + Hostname +
        "\r\nConnection: close\r\n\r\n");
    });

  while (!Finished)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

}
