#include <Http/Server.hpp>
#include <String/String.hpp>
#include <iostream>
#include <fstream>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, const char** argv)
{
  int Port = 80;

  if (argc > 1)
  {
    Port = std::atoi(argv[1]);
  }

  dl::http::Server Server(Port, 1, 1);

  Server.Register(
    dl::http::RequestType::eGet,
    "/",
    [] (const dl::http::Request& Request)
    {
      return dl::http::Responce::GenerateResponce(
        dl::http::Responce::Status::eOk,
        "text/html",
        "<html><body><h1>Web socket SERVER!</h1></body></html>");
    });

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
return 0;
}
