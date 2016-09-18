
#include <Tcp/Server.hpp>
#include <iostream>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  dl::tcp::Server tcpServer(8080);

  std::vector<std::shared_ptr<dl::tcp::Session>> Sessions;
  tcpServer.GetNewSessionSignal().Connect(
    [&Sessions] (auto pSession)
    {
      Sessions.push_back(pSession);
    });

  std::cout << "yo " << std::endl;

  while (true)
  {
  }
  return 0;
}
