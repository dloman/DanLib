
#include <Tcp/Client.hpp>
#include <iostream>
#include <array>

using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  cout << "Client connecting on localhost 8080" << endl;

  dl::tcp::Client Client;

  Client.GetOnRxSignal().Connect([] (auto Bytes) { cout << Bytes << endl;});

  Client.GetOnDisconnectSignal().Connect(
    [] { cout << "server has disconnected" << endl;});

  while (true)
  {
    Client.Write(to_string(chrono::system_clock::now().time_since_epoch().count()));
    //std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  return 0;
}
