
#include <Tcp/Server.hpp>
#include <iostream>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  dl::tcp::Server TcpServer(8080, 2, 2);

  std::vector<std::shared_ptr<dl::tcp::Session>> Sessions;

  TcpServer.GetNewSessionSignal().Connect(
    [&Sessions] (auto pSession)
    {
      pSession->GetOnRxSignal().Connect(
        [pSession] (const std::string& Bytes)
        {
          pSession->Write("Recived bytes = " + Bytes);
          std::cout << Bytes << std::endl;
        });

      pSession->GetOnDisconnectSignal().Connect(
        [] { std::cout << "Disconnect" << std::endl; });

      Sessions.push_back(pSession);
    });

  std::cout << "yo " << std::endl;

  while (true)
  {
  }
  return 0;
}
