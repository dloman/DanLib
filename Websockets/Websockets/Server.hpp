//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Dan Loman
// 2016-09-14
//
// Description:
//   This is a ws server
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#pragma once
#include <Websockets/Session.hpp>

#include <Signal/Signal.hpp>

#include <asio/io_service.hpp>
#include <asio/ip/tcp.hpp>

#include <thread>

namespace dl::ws
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

      std::shared_ptr<dl::ws::Session> GetSession();

      using NewSessionSignal = dl::Signal<std::shared_ptr<dl::ws::Session>>;

      const NewSessionSignal& GetNewSessionSignal() const;

      const dl::Signal<const boost::system::error_code&>& GetErrorSignal() const;

      const size_t GetConnectionCount() const;

      void Write(const std::string& Bytes, dl::ws::DataType DataType);

    private:

      void StartWorkerThreads(
        boost::asio::io_service& IoService,
        unsigned NumberOfThreads);

      void StartAccept();

      void OnAccept(const boost::system::error_code& Error);

    private:

      boost::asio::io_service mIoService;

      boost::asio::ip::tcp::acceptor mAcceptor;

      boost::asio::io_service mCallbackService;

      std::shared_ptr<boost::asio::io_service::work> mpNullWork;

      std::shared_ptr<dl::ws::Session> mpNewSession;

      std::vector<std::shared_ptr<dl::ws::Session>> mActiveSessions;

      std::vector<std::thread> mThreads;

      NewSessionSignal mSignalNewSession;

      dl::Signal<const boost::system::error_code&> mErrorSignal;

      std::mutex mMutex;
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  inline
  const Server::NewSessionSignal& Server::GetNewSessionSignal() const
  {
    return mSignalNewSession;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  inline
  const size_t Server::GetConnectionCount() const
  {
    return mActiveSessions.size();
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  inline
  const dl::Signal<const boost::system::error_code&>& Server::GetErrorSignal() const
  {
    return mErrorSignal;
  }
}
