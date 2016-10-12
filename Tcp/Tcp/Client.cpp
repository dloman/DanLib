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
  mMutex(),
  mThreads(),
  mIsRunning(false),
  mConnected(false),
  mpNullWork(nullptr)
{
  mpNullWork = std::make_shared<asio::io_service::work> (mCallbackService);

  StartWorkerThreads(mCallbackService, 1);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Client::~Client()
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
void Client::StartWorkerThreads(
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
void Client::Connect()
{
  std::lock_guard<std::recursive_mutex> Lock(mMutex);

  mSession.emplace(mIoService, mCallbackService);

  mSession->GetOnRxSignal().Connect(
    [] (const std::string Bytes)
    {
      std::cout << "Recived: " << Bytes << std::endl;
    });

  mSession->GetOnDisconnectSignal().Connect(
    [this] (const unsigned long SessionId)
    {
      mSession = std::experimental::nullopt;
    });

  if (!mConnected)
  {
    asio::ip::tcp::resolver::query Query(mHostname, std::to_string(mPort));

    mResolver.async_resolve(
      Query,
      [this]
      (const asio::error_code& Error, asio::ip::tcp::resolver::iterator iEndpoint)
      {
        OnResolve(Error, iEndpoint);
      });

    StartWorkerThreads(mIoService, 1);

    mConnected = true;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Client::Connect(const std::string& Hostname, const unsigned Port)
{
  std::lock_guard<std::recursive_mutex> Lock(mMutex);

  mHostname = Hostname;

  mPort = Port;

  Connect();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Client::OnResolve(
  const asio::error_code& Error,
  asio::ip::tcp::resolver::iterator iEndpoint)
{
  if (!Error)
  {
    socket_.async_connect(iEndpoint,
      [this] (asio::error_code& Error, asio::ip::tcp::resolver::iterator iEndpoint)
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
void Client::OnConnect(
  const asio::error_code& Error,
  asio::ip::tcp::resolver::iterator iEndpoint)
{
  if (!Error)
  {
    std::cout << "CONNECTION !!!!!" << std::endl;
    mSession->Start();
  }
  else if (iEndpoint != asio::ip::tcp::resolver::iterator())
  {
    socket_.async_connect(iEndpoint,
      [this] (asio::error_code& Error, asio::ip::tcp::resolver::iterator iEndpoint)
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
