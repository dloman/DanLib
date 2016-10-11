
#include <Tcp/Server.hpp>
#include <iostream>

using namespace std;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  dl::tcp::Server TcpServer(8080, 2, 2);

  cout << "Server Listening on 8080" << endl;

  TcpServer.GetNewSessionSignal().Connect(
    [] (std::weak_ptr<dl::tcp::Session> pWeakSession)
    {
      auto pSession = pWeakSession.lock();
      cout << "Connect!!!! " << endl;
      if (pSession)
      {
        pSession->GetOnRxSignal().Connect(
          [pWeakSession]
          (const std::string& Bytes)
          {
            std::shared_ptr<dl::tcp::Session> pSession = pWeakSession.lock();

            if (pSession)
            {
              pSession->Write("Recived bytes = " + Bytes);
              std::cout << Bytes << std::endl;
            }

          });

        pSession->GetOnDisconnectSignal().Connect(
          [] (const unsigned SessionId)
          {
            cout << "Session Id " << SessionId << " Disconnected" << endl;
          });

      }
    });

  while (true)
  {
    this_thread::sleep_for(chrono::seconds(4));
  }
  return 0;
}
