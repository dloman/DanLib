
#include <Tcp/Client.hpp>
#include <iostream>
#include <array>

using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  cout << "Client connecting on localhost 8080" << endl;

  dl::tcp::Client Client("localhost", 80);

  Client.GetOnRxSignal().Connect([] (const auto& Bytes) { cout << Bytes << endl;});

  Client.GetOnDisconnectSignal().Connect(
    [] { cout << "server has disconnected" << endl;});

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    Client.Write(
      std::string("GET / HTTP/1.1\r\nUser-Agent: Wget/1.17.1 (linux-gnu)\r\nAccept: */*\r\n") +
      "Accept-Encoding: identity\r\nHost: localhost:8080\r\n" +
      std::string("Connection: Keep-Alive\r\n"));

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  return 0;
}
