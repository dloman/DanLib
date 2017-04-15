#include <Tcp/Server.hpp>
#include <String/String.hpp>
#include <iostream>
#include <fstream>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static std::string ParseRequest(const std::string& Request)
{
  auto InputData = dl::Split(Request);

  auto FirstLine = dl::Split(InputData[0]);

  return FirstLine[0], FirstLine[1];
}

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
    dl::tcp::Server TcpServer(Port, 1, 1);

    std::cout << "Server Listening " << std::endl;

    std::ifstream WebPageStream("WebPage.html");

    std::string WebPage, line;

    while (std::getline(WebPageStream, line))
    {
      WebPage += line;
    }

    TcpServer.GetNewSessionSignal().Connect(
      [&WebPage] (std::weak_ptr<dl::tcp::Session> pWeakSession)
      {
        auto pSession = pWeakSession.lock();
        if (pSession)
        {
          pSession->GetOnRxSignal().Connect(
            [pWeakSession, &WebPage]
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

                  std::cout <<WebPage << std::endl;
                  Stream <<
                    "HTTP/1.1 200 OK\r\n" <<
                    "Date: Sun, 18 Oct 2009 08:56:53 GMT\r\n" <<
                    "Server: Apache/2.2.14 (Win32)\r\n" <<
                    "Last-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\n" <<
                    "ETag: \"10000000565a5-2c-3e94b66c2e680\"\r\n" <<
                    "Accept-Ranges: bytes\r\n" <<
                    "Content-Length: " + std::to_string(WebPage.size()) + "\r\n" <<
                    "Connection: close\r\n" <<
                    "Content-Type: text/html\r\n" <<
                    "X-Pad: avoid browser bug\r\n\r\n" <<
                    WebPage << "\r\n\r\n";

                  std::cout << Stream.str().size() << std::endl;

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
  }
  catch(const std::exception& Exception)
  {
    std::cerr << "ERROR: " << Exception.what() << std::endl;
  }
    return 0;
}
