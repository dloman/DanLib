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
    try
    {
      for (
        auto iEndpoint = GetEndpoint();
        iEndpoint != asio::ip::tcp::resolver::iterator();
        ++iEndpoint)
      {
        try
        {
          return Connect(*iEndpoint);
        }
        catch (const std::exception& Exception)
        {
          mSignalConnectionError(Exception.what());
        }
      }

      StartWorkerThreads(mIoService, 1);

      mConnected = true;

    }
    catch (const std::exception& Exception)
    {
      mSignalConnectionError(Exception.what());
    }
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
void Client::Connect(const asio::ip::tcp::socket::endpoint_type& Endpoint)
{
  asio::deadline_timer Timer(mIoService);

  //Timer.expires_from_now(std::chrono::seconds(5));

  mSession->GetSocket().async_connect(
    Endpoint,
    [this] (const asio::error_code& Error) { std::cout << "ya" << std::endl; OnConnect(Error); });

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Client::OnConnect(const asio::error_code& Error)
{
  std::cerr << "onconnect: " << Error << std::endl;
  if (Error)
  {
    std::cerr << "ERROR: " << Error << std::endl;
  }
  else
  {
    mSession->Start();
    std::cerr << "starting: " << Error << std::endl;
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

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
asio::ip::tcp::resolver::iterator Client::GetEndpoint() const
{
  asio::io_service IoService;

  asio::ip::tcp::resolver Resolver(IoService);

  asio::ip::tcp::resolver::query Query(mHostname, std::to_string(mPort));

  auto iResolver = Resolver.resolve(Query);

  if (iResolver == asio::ip::tcp::resolver::iterator())
  {
    throw std::runtime_error("Cannot resolve hostname.");
  }
  return iResolver;
}
