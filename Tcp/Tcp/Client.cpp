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
  mSession(),
  mEndpoint(asio::ip::address::from_string(Hostname), Port),
  mMutex(),
  mThreads(),
  mIsRunning(false),
  mConnected(false),
  mpNullWork(nullptr)
{
  StartWorkerThreads(mIoService, 1);

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
    [this] (const std::string Bytes)
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
    mSession->GetSocket().async_connect(
      mEndpoint,
      [this] (const asio::error_code& Error)
      {
        if (!Error)
        {
          std::cout << "Connection success!" << std::endl;
          mConnected = true;
        }
        else
        {
          std::cerr << "Error: " << Error << std::endl;
        }
      });
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Client::Connect(const std::string& Hostname, const unsigned Port)
{
  std::lock_guard<std::recursive_mutex> Lock(mMutex);

  mEndpoint =
    asio::ip::tcp::endpoint(asio::ip::address::from_string(Hostname), Port);
  Connect();
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
