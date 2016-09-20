
#include <Tcp/Server.hpp>
#include <iostream>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  dl::tcp::Server TcpServer(8080, 4);

  std::vector<std::shared_ptr<dl::tcp::Session>> Sessions;

  TcpServer.GetNewSessionSignal().Connect(
    [&Sessions] (auto pSession)
    {
      pSession->GetOnRxSignal().Connect(
        [] (const std::string& Bytes) {std::cout << Bytes << std::endl;});

      Sessions.push_back(pSession);
    });

  std::cout << "yo " << std::endl;

  while (true)
  {
  }
  return 0;
}
