#include <Websockets/Client.hpp>
#include <Websockets/Session.hpp>
#include <iostream>
#include <array>

using namespace std;
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>

//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  cout << "Client connecting on localhost 8181" << endl;

	ssl::context SslContext{ssl::context::sslv23_client};

  dl::ws::Client Client(SslContext);

  std::atomic<bool> Connected(false);

  Client.GetOnRxSignal().Connect([] (auto Bytes) { cout << Bytes << endl;});

  Client.GetOnDisconnectSignal().Connect(
    [&Connected]
    {
      Connected = false;

      cout << "server has disconnected" << endl;
    });

  Client.GetErrorSignal().Connect(
    [] (const std::string& Error)
    {
      cout << "ERROR: " << Error << endl;
    });

  Client.GetConnectionSignal().Connect([&Connected]
  {
    Connected = true;
  });

  Client.Connect();

  while (true)
  {
    if (Connected)
    {
      Client.Write(
        "client time = " +
        to_string(chrono::system_clock::now().time_since_epoch().count()) + "\n",
        dl::ws::DataType::eText);

      Client.Write(
        "client time = " +
        to_string(chrono::system_clock::now().time_since_epoch().count()) + "\n",
        dl::ws::DataType::eText);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  return 0;
}
