#include "Client.hpp"

using dl::tcp::Client;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Client::Client()
: Client("localhost", 8080)
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Client::Client(const std::string& Hostname, const unsigned Port)
: mIoService(),
  mCallbackService(),
  mResolver(mIoService),
  mSession(),
  mHostname(Hostname),
  mPort(Port),
  mConnectionMutex(),
  mThreads(),
  mIsRunning(true),
  mpNullIoWork(std::make_unique<asio::io_service::work> (mIoService)),
  mpNullCallbackWork(std::make_unique<asio::io_service::work> (mCallbackService))
{
  StartWorkerThreads(mCallbackService, 1);

  Connect();

  StartWorkerThreads(mIoService, 1);
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

  mSession->GetOnDisconnectSignal().Connect(
    [this]
    {
      std::cout << "disconnect" << std::endl;
      Connect();
    });

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
    asio::async_connect(
      mSession->GetSocket(),
      iEndpoint,
      [this]
      (const asio::error_code& Error, asio::ip::tcp::resolver::iterator iEndpoint)
      {
        OnConnect(Error, iEndpoint);
      });
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
void Client::Write(const std::string& Bytes)
{
  if (mSession)
  {
    mSession->Write(Bytes);
  }
}
