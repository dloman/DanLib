#include "Client.hpp"

using dl::tcp::Client;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Client::Client(
  const std::string& Hostname,
  const unsigned Port,
  const unsigned NumberOfIoThreads,
  const unsigned NumberOfCallbackThreads)
: mIoService(),
  mCallbackService(),
  mResolver(mIoService),
  mSession(),
  mTimer(mIoService),
  mHostname(Hostname),
  mPort(Port),
  mConnectionMutex(),
  mThreads(),
  mIsRunning(true),
  mpNullIoWork(std::make_unique<asio::io_service::work> (mIoService)),
  mpNullCallbackWork(std::make_unique<asio::io_service::work> (mCallbackService))
{
  StartWorkerThreads(mCallbackService, NumberOfCallbackThreads);

  Connect();

  StartWorkerThreads(mIoService, NumberOfIoThreads);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Client::~Client()
{
  mIsRunning = false;

  mIoService.stop();

  mpNullIoWork.reset();

  mpNullCallbackWork.reset();

  for (auto& Thread : mThreads)
  {
    Thread.join();
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void dl::tcp::Client::StartWorkerThreads(
  asio::io_service& IoService,
  unsigned NumberOfThreads)
{
  for (unsigned i = 0u; i < NumberOfThreads; ++i)
  {
    mThreads.emplace_back([&IoService] { IoService.run(); });
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Client::Connect()
{
  mSession.emplace(mIoService, mCallbackService);

  mSession->GetOnRxSignal().Connect(
    [this] (const std::string Bytes)
    {
      mSignalOnRx(Bytes);
    });

  mSession->GetOnDisconnectSignal().Connect([this] { Connect(); });

  asio::ip::tcp::resolver::query Query(mHostname, std::to_string(mPort));

  mResolver.async_resolve(
    Query,
    [this]
    (const asio::error_code& Error, asio::ip::tcp::resolver::iterator iEndpoint)
    {
      OnResolve(Error, iEndpoint);
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Client::OnResolve(
  const asio::error_code& Error,
  asio::ip::tcp::resolver::iterator iEndpoint)
{
  if (!Error)
  {
    mTimer.expires_from_now(std::chrono::seconds(2));

    asio::async_connect(
      mSession->GetSocket(),
      iEndpoint,
      [this]
      (const asio::error_code& Error, asio::ip::tcp::resolver::iterator iEndpoint)
      {
        OnConnect(Error, iEndpoint);
      });

    mTimer.async_wait([this] (const asio::error_code& Error) { OnTimeout(Error); });
  }
  else
  {
    mSignalConnectionError(Error.message());
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Client::OnConnect(
  const asio::error_code& Error,
  asio::ip::tcp::resolver::iterator iEndpoint)
{
  if (!Error)
  {
    mSession->Start();

    mTimer.cancel();
  }
  else if (iEndpoint != asio::ip::tcp::resolver::iterator())
  {
    asio::async_connect(
      mSession->GetSocket(),
      iEndpoint,
      [this] (const asio::error_code& Error, asio::ip::tcp::resolver::iterator iEndpoint)
      {
        OnConnect(Error, ++iEndpoint);
      });
  }
  else
  {
    mSignalConnectionError(Error.message());
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Client::OnTimeout(const asio::error_code& Error)
{
  if (!Error)
  {
    Connect();
  }
  else if (Error != asio::error::operation_aborted)
  {
    mSignalConnectionError(Error.message());
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Client::Write(const std::string& Bytes)
{
  if (mSession)
  {
    mSession->Write(Bytes);
  }
}
