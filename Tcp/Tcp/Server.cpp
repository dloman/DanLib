//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Dan Loman
// 2016-09-14
//
// Description:
//   This is a tcp server
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "Server.hpp"
#include <iostream>

using dl::tcp::Server;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Server::Server(
  unsigned short Port,
  unsigned NumberOfIoThreads,
  unsigned NumberOfCallbackThreads)
  : mIoService(),
    mAcceptor(mIoService, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), Port)),
    mCallbackService(),
    mpNullWork(nullptr),
    mpNewSession(nullptr),
    mActiveSessions(),
    mThreads()
{
  mAcceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));

  StartAccept();

  StartWorkerThreads(mIoService, NumberOfIoThreads);

  mpNullWork = std::make_shared<asio::io_service::work> (mCallbackService);

  StartWorkerThreads(mCallbackService, NumberOfCallbackThreads);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Server::StartWorkerThreads(
  asio::io_service& IoService,
  unsigned NumberOfThreads)
{
  for (unsigned i = 0u; i < NumberOfThreads; ++i)
  {
    mThreads.emplace_back([this, &IoService] { IoService.run(); });
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Server::~Server()
{
  mIoService.stop();

  mpNullWork.reset();

  for (auto& Thread : mThreads)
  {
    Thread.join();
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Server::StartAccept()
{
  mpNewSession =
    std::make_shared<dl::tcp::Session>(mIoService, mCallbackService);

  mAcceptor.async_accept(
    mpNewSession->GetSocket(),
    [this] (asio::error_code Error)
    {
      OnAccept(Error);
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Server::OnAccept(asio::error_code& Error)
{
  if (!Error)
  {
    mpNewSession->Start();
    mActiveSessions.push_back(mpNewSession);

    mpNewSession->GetOnDisconnectSignal().Connect(
      [this] (const unsigned long SessionId)
      {
        mActiveSessions.erase(
          std::remove_if(
            mActiveSessions.begin(),
            mActiveSessions.end(),
            [&SessionId] (std::shared_ptr<dl::tcp::Session> pSession)
            {
              return pSession->GetSessionId() == SessionId;
            }),
          mActiveSessions.end());
      });

    mSignalNewSession(mpNewSession);
  }
  else
  {
    std::cerr
      << "ERROR: Error Accepting Session Line:"
      << __LINE__ << " File: " << __FILE__  << std::endl;
  }

  StartAccept();
}
