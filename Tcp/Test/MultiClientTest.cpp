
#include <Tcp/Client.hpp>
#include <iostream>
#include <array>

using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  cout << "Client connecting on localhost 8181" << endl;

  for (int i = 0; i < 1000; ++i)
  {
    std::array<dl::tcp::Client<dl::tcp::Session>, 100> Clients;

    for (auto& Client : Clients)
    {
      Client.GetOnRxSignal().Connect
        ([] (const auto& Bytes) { cout << Bytes << endl;});
    }

    for (auto& TcpClient : Clients)
    {
      TcpClient.Write("foo");
    }
  }

  return 0;
}
