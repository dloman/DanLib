
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
    std::vector<dl::tcp::Client<dl::tcp::Session>> Clients;

    //for (int i = 0; i < 100; ++i)
    //{
      //Clients.emplace_back(
        //dl::tcp::ClientSettings<dl::tcp::Session>{
          //.mOnRxCallback = [] (const string& Bytes) { cout << Bytes << endl;}});
    //}

    for (auto& TcpClient : Clients)
    {
      TcpClient.Write("foo");
    }
  }

  return 0;
}
