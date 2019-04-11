//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Dan Loman
// 2016-09-14
//
// Description:
//   This is a tcp server
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#pragma once
#include <Tcp/Session.hpp>

#include <Signal/Signal.hpp>

#include <asio/io_service.hpp>
#include <asio/ip/tcp.hpp>

#include <thread>

namespace dl::tcp
{
  struct ServerSettings
  {
    unsigned short mPort = 8080;

    unsigned mNumberOfIoThreads = 1;

    unsigned mNumberOfCallbackThreads = 1;

    std::function<void(std::shared_ptr<dl::tcp::Session>)> mOnNewSessionCallback;

    std::function<void(const asio::error_code&)> mErrorCallback;
  };

  class Server
  {
    public:

      Server(const ServerSettings&);

      ~Server();

      Server(const Server& Other) = delete;

      Server& operator = (const Server& Rhs) = delete;

      using NewSessionSignal = dl::Signal<std::shared_ptr<dl::tcp::Session>>;

      const size_t GetConnectionCount() const;

      void Write(const std::string& Bytes);

    private:

      void StartWorkerThreads(
        asio::io_service& IoService,
        unsigned NumberOfThreads);

      void StartAccept();

      void OnAccept(asio::error_code& Error);

    private:

      asio::io_service mIoService;

      asio::ip::tcp::acceptor mAcceptor;

      asio::io_service mCallbackService;

      std::shared_ptr<asio::io_service::work> mpNullWork;

      std::shared_ptr<dl::tcp::Session> mpNewSession;

      std::vector<std::shared_ptr<dl::tcp::Session>> mActiveSessions;

      std::vector<std::thread> mThreads;

      NewSessionSignal mSignalNewSession;

      dl::Signal<const asio::error_code&> mErrorSignal;

      std::mutex mMutex;
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  inline
  const size_t Server::GetConnectionCount() const
  {
    return mActiveSessions.size();
  }
}
