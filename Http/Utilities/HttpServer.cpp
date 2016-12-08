#include <Tcp/Server.hpp>
#include <String/String.hpp>
#include <iostream>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static std::string ParseRequest(const std::string& Request)
{
  auto InputData = dl::Split(Request);

  if (InputData[0].find("GET") != std::string::npos)
  {
    std::cout << "GET!!" << std::endl;
  }
  else if (InputData[0].find("POST") != std::string::npos)
  {
    std::cout << "POST!!" << std::endl;
  }
  else
  {
    std::cerr << "fuckkkk" << std::endl;
  }
  std::cout << "Request = " << Request << std::endl;
  return "";
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  dl::tcp::Server TcpServer(8080, 2, 2);

  std::cout << "Server Listening on 8080" << std::endl;

  TcpServer.GetNewSessionSignal().Connect(
    [] (std::weak_ptr<dl::tcp::Session> pWeakSession)
    {
      auto pSession = pWeakSession.lock();
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

                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                ParseRequest(Bytes);

                std::stringstream Stream;

                Stream <<
                  "HTTP/1.1 200 OK\r\n" <<
                  "Date: Sun, 18 Oct 2009 08:56:53 GMT\r\n" <<
                  "Server: Apache/2.2.14 (Win32)\r\n" <<
                  "Last-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\n" <<
                  "ETag: \"10000000565a5-2c-3e94b66c2e680\"\r\n" <<
                  "Accept-Ranges: bytes\r\n" <<
                  "Content-Length: 44\r\n" <<
                  "Connection: close\r\n" <<
                  "Content-Type: text/html\r\n" <<
                  "X-Pad: avoid browser bug\r\n\r\n" <<
                  "<html><body><h1>It works!</h1></body></html>\r\n\r\n";
                pSession->Write(Stream.str());

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
            std::cout
              << "Session Id " << SessionId
              << " Disconnected" << std::endl;
          });

      }
    });

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return 0;
}
