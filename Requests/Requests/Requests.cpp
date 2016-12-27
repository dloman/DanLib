#include "Requests.hpp"

#include <Tcp/Client.hpp>
#include <condition_variable>

using namespace std::literals;

namespace dl::request
{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  static std::pair<std::string, std::string> GetHostnameAndPath(
    const std::string& Url)
  {
    std::pair<std::string, std::string> HostnamePath;

    auto iProtocol = Url.find("://");

    std::string Hostname;

    if (iProtocol == std::string::npos)
    {
      Hostname = Url;
    }
    else
    {
      Hostname = Url.substr(iProtocol + 3) ;
    }



    if (auto iPath = Hostname.find("/"); iPath == std::string::npos)
    {
      return {Hostname, "/"s};
    }
    else
    {
      return {Hostname.substr(0, iPath), Hostname.substr(iPath)};
    }
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  static std::string GetRequestString(
    const std::string& Hostname,
    const std::string& Path)
  {
    std::stringstream Request;

    Request
      << Path << " HTTP/1.1\r\n"
      << "Host: " << Hostname << "\r\n"
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
        dl::tcp::Client Client(Url, Port, 1, 1);

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
              Client.Write(Data);
              FirstTime = false;
            }
          });

        std::unique_lock<std::mutex> Lock(Mutex);

        Condition.wait(Lock);
        //if (Condition.wait_for(Lock, 3s) == std::cv_status::timeout)
        //{
        //try
        //{
        //throw std::logic_error("Time out");
        //}
        //catch (const std::exception& Exception)
        //{
        //Promise.set_exception(std::current_exception());
        //}
        //}
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
    auto [Hostname, Path] = GetHostnameAndPath(Url);

    return MakeRequest(Hostname, "GET " + GetRequestString(Hostname, Path), Port);
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  std::future<dl::http::Responce> Post(
    const std::string& Url,
    std::unordered_map<std::string, std::string> Data,
    const unsigned Port)
  {
    auto [Hostname, Path] = GetHostnameAndPath(Url);

    return MakeRequest(
      Hostname,
      "POST " + GetRequestString(Hostname, Path) + FormatPostData(Data),
      Port);
  }
}
