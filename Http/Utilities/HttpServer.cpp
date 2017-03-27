#include <Http/Server.hpp>
#include <Http/Request.hpp>
#include <iostream>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, const char** argv)
{
  int Port = 80;

  if (argc > 1)
  {
    Port = std::atoi(argv[1]);
  }

  try
  {
    dl::http::Server Server(Port, 1, 1);

    Server.Register(
      dl::http::RequestType::eGet,
      "/",
      [] (const dl::http::Request& Request)
      {
        return dl::http::Responce::GenerateResponce(
          dl::http::Responce::Status::eOk,
          "text/html",
          "<html><body><h1>It works!</h1></body></html>");
      });

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
  catch(const std::exception& Exception)
  {
    std::cerr << "ERROR: " << Exception.what() << std::endl;
  }
    return 0;
}
