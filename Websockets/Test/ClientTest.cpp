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

  pClient->GetOnRxSignal().Connect([] (auto Bytes) { cout << Bytes << endl;});

  pClient->GetOnDisconnectSignal().Connect(
    []
    {
      cout << "server has disconnected" << endl;
    });

  pClient->GetErrorSignal().Connect(
    [] (const std::string& Error)
    {
      cout << "ERROR: " << Error << endl;
    });

  pClient->Connect();

  while (true)
  {
    pClient->Write(
      "client time = " +
      to_string(chrono::system_clock::now().time_since_epoch().count()) + "\n",
      dl::ws::DataType::eText);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  return 0;
}
