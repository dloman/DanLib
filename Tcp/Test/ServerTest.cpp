
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
      cout << "Connect!!!! " << pSession->GetSessionId() << endl;
      if (pSession)
      {
        pSession->GetOnRxSignal().Connect(
          [pWeakSession]
          (const std::string& Bytes)
          {
            std::shared_ptr<dl::tcp::Session> pSession = pWeakSession.lock();

            if (pSession)
            {
              try
              {
                pSession->Write("Recived bytes = " + Bytes);
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
          [SessionId] ()
          {
            cout
              << "Session Id " << SessionId
              << " Disconnected" << endl;
          });

      }
    });

  while (true)
  {
    this_thread::sleep_for(chrono::seconds(4));
  }
  return 0;
}
