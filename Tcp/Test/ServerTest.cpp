
#include <Tcp/Server.hpp>
#include <iostream>

using namespace std;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  dl::tcp::Server TcpServer(dl::tcp::ServerSettings{
    .mPort = 8181,
    .mNumberOfIoThreads = 2,
    .mNumberOfCallbackThreads = 2,
    .mOnNewSessionCallback =
      [] (std::shared_ptr<dl::tcp::Session> pSession)
      {
        cout << "Connect!!!! " << pSession->GetSessionId() << endl;
        if (pSession)
        {
          pSession->GetOnRxSignal().Connect(
            [pWeakSession= std::weak_ptr<dl::tcp::Session>(pSession)]
            (const std::string& Bytes)
            {
              if (auto pSession = pWeakSession.lock())
              {
                try
                {
                  pSession->Write("Server recived bytes = " + Bytes);
                  std::cout << Bytes << std::endl;
                }
                catch(std::exception& Exception)
                {
                  std::cerr << "ERROR: " << Exception.what();
                }
              }
            });

          auto SessionId = pSession->GetSessionId();

          pSession->GetOnDisconnectSignal().Connect(
            [SessionId]
            {
              cout
                << "Session Id " << SessionId
                << " Disconnected" << endl;
            });

        }
      }});

  while (true)
  {
    TcpServer.Write(
      "\nServer time = " + std::to_string(chrono::system_clock::now().time_since_epoch().count()));

    this_thread::sleep_for(chrono::milliseconds(10));
  }
  return 0;
}
