
#include <Tcp/SslClient.hpp>
#include <iostream>
#include <array>

using namespace std;
using namespace std::literals;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  std::atomic<bool> Finished = false;

  cout << "Client connecting to example.com" << endl;

  dl::tcp::SslClient Client("example.com", 443);

  Client.GetOnRxSignal().Connect([] (const auto& Bytes) { cout << "Bytes = " << Bytes << endl;});

  Client.GetOnDisconnectSignal().Connect(
    [&] { std::cout << "disconnected" << std::endl; Finished = true;});

  Client.GetConnectionSignal().Connect([&Client]
    {
      cout << "server has connected" << endl;

      Client.Write(
        "GET / HTTP/1.1\r\nUser-Agent: Wget/1.17.1 (linux-gnu)\r\nAccept: */*\r\n"s +
        "Accept-Encoding: identity\r\nHost: example.com:443\r\n" +
        "Connection: close\r\n\r\n"s);
    });

  Client.GetConnectionErrorSignal().Connect(
    [] (const std::string& Error)
    {
      std::cerr << "ERROR: " << Error << std::endl;
    });

  while (!Finished)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

}
