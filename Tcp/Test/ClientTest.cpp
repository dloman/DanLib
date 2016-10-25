
#include <Tcp/Client.hpp>
#include <iostream>
#include <array>

using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  cout << "Client connecting on localhost 8080" << endl;

  for (int i = 0; i < 100; ++i)
  {
    std::array<dl::tcp::Client, 100> Clients;

    for (auto& Client : Clients)
    {
      Client.GetOnRxSignal().Connect([] (auto Bytes) { cout << Bytes << endl;});
    }

    for (auto& TcpClient : Clients)
    {
      TcpClient.Write("foo");
    }
  }

  return 0;
}
