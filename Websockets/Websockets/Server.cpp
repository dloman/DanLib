//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Dan Loman
// 2016-09-14
//
// Description:
//   This is a ws server
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "Server.hpp"

using dl::ws::Server;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Server::Server(
  unsigned short Port,
  unsigned NumberOfIoThreads,
  unsigned NumberOfCallbackThreads)
  : mIoService(),
    mAcceptor(
      mIoService,
      boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), Port)),
    mCallbackService(),
    mpNullWork(nullptr),
    mpNewSession(nullptr),
    mActiveSessions(),
    mThreads(),
    mMutex()
{
  mAcceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));

  StartAccept();

  StartWorkerThreads(mIoService, NumberOfIoThreads);

  mpNullWork =
    std::make_shared<boost::asio::io_service::work> (mCallbackService);

  StartWorkerThreads(mCallbackService, NumberOfCallbackThreads);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Server::StartWorkerThreads(
  boost::asio::io_service& IoService,
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
  mpNewSession = dl::ws::Session::Create(mIoService, mCallbackService);

  mAcceptor.async_accept(
    mpNewSession->GetSocket(),
    [this] (const boost::system::error_code& Error)
    {
      OnAccept(Error);
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Server::OnAccept(const boost::system::error_code& Error)
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
            [SessionId] (std::shared_ptr<dl::ws::Session> pSession)
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
void Server::Write(const std::string& Bytes, dl::ws::DataType DataType)
{
  std::lock_guard<std::mutex> Lock(mMutex);

  for (const auto pSession : mActiveSessions)
  {
    pSession->Write(Bytes, DataType);
  }
}
