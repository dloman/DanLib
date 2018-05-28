#include <Websockets/Client.hpp>
#include <Websockets/Session.hpp>
#include <iostream>
#include <array>

using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  cout << "Client connecting on localhost 8181" << endl;

  auto pClient = dl::ws::Client::Create();

  std::atomic<bool> Connected(false);

  pClient->GetOnRxSignal().Connect([] (auto Bytes) { cout << Bytes << endl;});

  pClient->GetOnDisconnectSignal().Connect(
    [&Connected]
    {
      Connected = false;

      cout << "server has disconnected" << endl;
    });

  pClient->GetErrorSignal().Connect(
    [] (const std::string& Error)
    {
      cout << "ERROR: " << Error << endl;
    });

  pClient->GetConnectionSignal().Connect([&Connected]
  {
    Connected = true;
  });

  pClient->Connect();

  while (true)
  {
    if (Connected)
    {
      pClient->Write(
        "client time = " +
        to_string(chrono::system_clock::now().time_since_epoch().count()) + "\n",
        dl::ws::DataType::eText);

      pClient->Write(
        "client time = " +
        to_string(chrono::system_clock::now().time_since_epoch().count()) + "\n",
        dl::ws::DataType::eText);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  return 0;
}
