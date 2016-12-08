#include <Http/Responce.hpp>
#include <Signal/Signal.hpp>
#include <Tcp/Client.hpp>
#include <condition_variable>
#include <future>

namespace dl::request
{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  std::string GetRequestString(const std::string& Url)
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
  std::future<dl::http::Responce> MakeRequest(
    const std::string& Url,
    const std::string& Data)
  {
    std::promise<dl::http::Responce> Promise;

    std::future<dl::http::Responce> Future = Promise.get_future();

    std::thread Thread(
      [Url, Data = std::move(Data), Promise = std::move(Promise)] () mutable
      {
        dl::tcp::Client Client(Url, 80);

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
            Client.Write(Data);
          });

        std::unique_lock<std::mutex> Lock(Mutex);

        Condition.wait(Lock);
      });

    Thread.detach();

    return std::move(Future);
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  std::future<dl::http::Responce> Get(const std::string& Url)
  {
    return MakeRequest(Url, "GET / " + GetRequestString(Url));
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  std::future<dl::http::Responce> Post(
    const std::string& Url,
    std::unordered_map<std::string, std::string> Data)
  {
    return MakeRequest(Url, "POST / " + GetRequestString(Url));
  }
}
