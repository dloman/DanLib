
#include <Tcp/Client.hpp>
#include <iostream>
#include <array>

using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  cout << "Client connecting on localhost 8181" << endl;

  dl::tcp::Client<dl::tcp::Session> Client(dl::tcp::ClientSettings<dl::tcp::Session>{
    .mOnRxCallback = [] (auto Bytes) { cout << Bytes << endl;},
    .mOnDisconnectCallback = [] { cout << "server has disconnected" << endl;}});

  while (true)
  {
    Client.Write(to_string(chrono::system_clock::now().time_since_epoch().count()));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  return 0;
}
