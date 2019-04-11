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

using dl::tcp::Server;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Server::Server(const dl::tcp::ServerSettings& Settings)
  : mIoService(),
    mAcceptor(
      mIoService,
      asio::ip::tcp::endpoint(asio::ip::tcp::v4(), Settings.mPort)),
    mCallbackService(),
    mpNullWork(nullptr),
    mpNewSession(nullptr),
    mActiveSessions(),
    mThreads(),
    mMutex()
{
  if (Settings.mOnNewSessionCallback)
  {
    mSignalNewSession.Connect(Settings.mOnNewSessionCallback);
  }

  if (Settings.mErrorCallback)
  {
    mErrorSignal.Connect(Settings.mErrorCallback);
  }

  mAcceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));

  StartAccept();

  StartWorkerThreads(mIoService, Settings.mNumberOfIoThreads);

  mpNullWork = std::make_shared<asio::io_service::work> (mCallbackService);

  StartWorkerThreads(mCallbackService, Settings.mNumberOfCallbackThreads);
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
  mpNewSession = dl::tcp::Session::Create(mIoService, mCallbackService);

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
    {
      std::lock_guard<std::mutex> Lock(mMutex);
      mActiveSessions.push_back(mpNewSession);
    }

    auto SessionId = mpNewSession->GetSessionId();

    mpNewSession->GetOnDisconnectSignal().Connect(
      [this, SessionId, pCurrentSession = mpNewSession] () mutable
      {
        std::lock_guard<std::mutex> Lock(mMutex);

        mActiveSessions.erase(
          std::remove_if(
            mActiveSessions.begin(),
            mActiveSessions.end(),
            [SessionId] (std::shared_ptr<dl::tcp::Session> pSession)
            {
              return pSession->GetSessionId() == SessionId;
            }),
        mActiveSessions.end());

        pCurrentSession = nullptr;
      });

    mSignalNewSession(mpNewSession);

    mpNewSession->Start();

  }
  else
  {
    mErrorSignal(Error);
  }

  StartAccept();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Server::Write(const std::string& Bytes)
{
  std::lock_guard<std::mutex> Lock(mMutex);

  for (const auto pSession : mActiveSessions)
  {
    pSession->Write(Bytes);
  }
}

