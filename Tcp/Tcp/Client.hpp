//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Dan Loman
// 2016-10-8
//
// Description:
//   This is a tcp client
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#pragma once
#include <Tcp/Session.hpp>

#include <Signal/Signal.hpp>

#include <asio/io_service.hpp>
#include <asio/ip/tcp.hpp>

//#include <optional>
#include <experimental/optional>

#include <thread>

namespace dl::tcp
{
  class Client
  {
    public:

      Client();

      Client(const std::string& Hostname, const unsigned Port);

      ~Client();

      Client(const Client& Other) = delete;

      Client& operator = (const Client& Rhs) = delete;

      void Connect();

      void Connect(const std::string& Hostname, const unsigned Port);

      void Write(const std::string& Bytes);

    private:

      void StartWorkerThreads(
        asio::io_service& IoService,
        unsigned NumberOfThreads);

      void Entry();

    private:

      asio::io_service mIoService;

      asio::io_service mCallbackService;

      std::experimental::optional<dl::tcp::Session> mSession;

      asio::ip::tcp::endpoint mEndpoint;

      std::recursive_mutex mMutex;

      std::vector<std::thread> mThreads;

      std::atomic<bool> mIsRunning;

      std::atomic<bool> mConnected;

      std::shared_ptr<asio::io_service::work> mpNullWork;

  };
}
