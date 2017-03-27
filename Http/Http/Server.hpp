#pragma once
#include <Tcp/Server.hpp>
#include <Http/RequestData.hpp>
#include <Http/Responce.hpp>
#include <string>
#include <type_traits>

namespace dl::http
{
  class Request;

  class Server
  {
    public:

      Server(
        const unsigned int Port = 80,
        const unsigned NumberOfIoThreads = 1,
        const unsigned NumberOfCallbackThreads = 1);

      template <typename CallableType>
      void Register(
        const RequestType& requestType,
        const std::string& url,
        CallableType&& callable)
      {
        mCallbacks[{requestType, url}] = std::forward<CallableType>(callable);
      }

    private:

      std::string ParseRequest(const dl::http::Request& Request);

      dl::tcp::Server mServer;

      std::unordered_map<
        RequestData,
        std::function<dl::http::Responce(const dl::http::Request&)>> mCallbacks;
  };
}
