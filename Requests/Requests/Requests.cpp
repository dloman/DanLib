#include "Requests.hpp"
#include <Tcp/Client.hpp>
#include <condition_variable>

namespace dl::request
{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  static std::string GetRequestString(const std::string& Url)
  {
    std::stringstream Request;

    Request
      << "HTTP/1.0\r\nHost: " << Url << "\r\n"
      << "Accept: */*\r\n"
      << "Accept-Encoding: text/plain\r\n"
      << "User-Agent: DanLib/0.0.1\r\n"
      << "Connection: close\r\n\r\n";

    return Request.str();
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  static std::string FormatPostData(
    const std::unordered_map<std::string, std::string>& Data)
  {
    std::string Post;

    for (auto& [Key, Value] : Data)
    {
      Post += Key + "=" + Value;
    }
    return Post;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  static std::future<dl::http::Responce> MakeRequest(
    const std::string& Url,
    const std::string& Data,
    const unsigned Port)
  {
    std::promise<dl::http::Responce> Promise;

    std::future<dl::http::Responce> Future = Promise.get_future();

    std::thread Thread(
      [Url, Port, Data = std::move(Data), Promise = std::move(Promise)] () mutable
      {
        dl::tcp::Client Client(Url, Port);

        std::mutex Mutex;

        std::condition_variable Condition;

        dl::http::Responce Responce;

        Client.GetOnRxSignal().Connect(
          [&] (const auto& Bytes)
          {
            if (Responce.AddBytes(Bytes))
            {
              Promise.set_value(Responce);

              Condition.notify_one();
            }
          });

        Client.GetConnectionErrorSignal().Connect(
          [&Promise] (const auto& Error)
          {
            try
            {
              throw std::logic_error(Error);
            }
            catch (const std::exception& Exception)
            {
              Promise.set_exception(std::current_exception());
            }
          });

        Client.GetConnectionSignal().Connect(
          [&Client, &Data]
          {
            static bool FirstTime = true;
            if (FirstTime)
            {
              std::cout << "Connected writing data" << std::endl;
              Client.Write(Data);
              FirstTime = false;
            }
          });

        std::unique_lock<std::mutex> Lock(Mutex);

        Condition.wait(Lock);
      });

    Thread.detach();

    return std::move(Future);
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  std::future<dl::http::Responce> Get(
    const std::string& Url,
    const unsigned Port)
  {
    return MakeRequest(Url, "GET / " + GetRequestString(Url), Port);
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  std::future<dl::http::Responce> Post(
    const std::string& Url,
    std::unordered_map<std::string, std::string> Data,
    const unsigned Port)
  {
    return MakeRequest(
      Url,
      "POST / " + GetRequestString(Url) + FormatPostData(Data),
      Port);
  }
}
