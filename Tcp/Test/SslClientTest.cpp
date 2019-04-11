
#include <Tcp/Client.hpp>
#include <iostream>
#include <array>

using namespace std;
using namespace std::literals;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  /*
  std::atomic<bool> Finished = false;

  cout << "Client connecting to example.com" << endl;

  dl::tcp::Client<dl::tcp::Session> Client(dl::tcp::ClientSettings<dl::tcp::Session>{
    .mHostname = "example.com",
    .mPort = 443,
    .mConnectionCallback =
      [&Client] (const auto& pSession)
      {
        cout << "server has connected" << endl;

        Client.Write(
          "Accept-Encoding: identity\r\nHost: example.com:443\r\n" +
          "Connection: close\r\n\r\n"s);
      },
    .mOnRxCallback = [] (auto Bytes) { cout << "Bytes = " << Bytes << endl;},
    .mConnectionErrorCallback = [] (const auto& Error) { cout << "Error = " << Error << endl;},
    .mOnDisconnectCallback = [&Finished] { cout << "server has disconnected" << endl; Finished = true;}});

  while (!Finished)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
*/
}
