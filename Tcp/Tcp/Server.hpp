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
  class Server
  {
    public:

      Server(
        unsigned short Port,
        unsigned NumberOfIoThreads = 1,
        unsigned NumberOfCallbackThreads = 1);

      ~Server();

      Server(const Server& Other) = delete;

      Server& operator = (const Server& Rhs) = delete;

      std::shared_ptr<dl::tcp::Session> GetSession();

      using NewSessionSignal = dl::Signal<std::shared_ptr<dl::tcp::Session>>;

      const NewSessionSignal& GetNewSessionSignal() const;

    private:

      void StartWorkerThreads(
        asio::io_service& IoService,
        unsigned NumberOfThreads);

      void StartAccept();

      void OnAccept(
        std::shared_ptr<dl::tcp::Session> pSession,
        asio::error_code& Error);

    private:

      asio::io_service mIoService;

      asio::ip::tcp::acceptor mAcceptor;

      asio::io_service mCallbackService;

      std::shared_ptr<asio::io_service::work> mpNullWork;

      std::vector<std::thread> mThreads;

      NewSessionSignal mSignalNewSession;
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  inline
  const Server::NewSessionSignal& Server::GetNewSessionSignal() const
  {
    return mSignalNewSession;
  }
}

