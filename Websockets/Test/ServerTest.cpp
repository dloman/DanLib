
#include <Websockets/Server.hpp>
#include <iostream>

using namespace std;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  dl::ws::Server WebsocketServer(8181, 2, 2);

  cout << "Server Listening on 8181" << endl;

  WebsocketServer.GetNewSessionSignal().Connect(
    [] (std::weak_ptr<dl::ws::Session> pWeakSession)
    {
    //std::cout << "woot connected" << std::endl;

      auto pSession = pWeakSession.lock();

      //cout << "Connect!!!! " << pSession->GetSessionId() << endl;

      if (pSession)
      {
        pSession->GetOnRxSignal().Connect(
          [pWeakSession]
          (const std::string& Bytes)
          {
            std::shared_ptr<dl::ws::Session> pSession = pWeakSession.lock();

            if (pSession)
            {
              std::cout << "Server recieved bytes = "  << Bytes << std::endl;
            }
          });

        auto SessionId = pSession->GetSessionId();

        pSession->GetOnDisconnectSignal().Connect(
          [SessionId]
          {
          //cout
          //<< "Session Id " << SessionId
          //<< " Disconnected" << endl;
          });

        pSession->GetSignalError().Connect(
         [] (const boost::system::error_code& ErrorCode, const std::string& Message)
         {
           std::cerr <<
             "Error: " <<
             ErrorCode.value() << " " <<
             ErrorCode.message() << " " <<
             ErrorCode << " " <<
             Message << std::endl;
         });
      }
    });

  while (true)
  {
    WebsocketServer.Write(
      "\nServer time = " +
      std::to_string(chrono::system_clock::now().time_since_epoch().count()),
      dl::ws::DataType::eText);

    std::cout << "number of active connection = " << WebsocketServer.GetConnectionCount() << std::endl;

    this_thread::sleep_for(chrono::milliseconds(1000));
  }
  return 0;
}
