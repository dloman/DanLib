//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Dan Loman
// 2016-10-8
//
// Description:
//   This is a tcp client
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#pragma once
#include <Signal/Signal.hpp>
#include <Tcp/SessionFactory.hpp>

#include <asio/io_service.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/basic_waitable_timer.hpp>

#include <thread>

namespace dl::tcp
{
  template <typename SessionType = dl::tcp::Session>
  struct ClientSettings
  {
    std::string mHostname = "localhost";

    unsigned mPort = 8181;

    unsigned mNumberOfIoThreads = 2;

    unsigned mNumberOfCallbackThreads = 2;

    std::function<void(const std::string&)> mOnRxCallback;

    std::function<void(const std::shared_ptr<SessionType>&)> mConnectionCallback;

    std::function<void(const std::string&)> mConnectionErrorCallback;

    std::function<void(void)> mOnDisconnectCallback;

  };

  template <typename SessionType = dl::tcp::Session>
  class Client
  {
    public:

      Client(const ClientSettings<SessionType>&);

      ~Client();

      Client(const Client& Other) = delete;

      Client& operator = (const Client& Rhs) = delete;

      void Write(const std::string& Bytes);

    private:

      void Connect();

      void StartConnect();

      void StartWorkerThreads(
        asio::io_service& IoService,
        unsigned NumberOfThreads);

      void Entry();

      void OnResolve(
        const asio::error_code& Error,
        asio::ip::tcp::resolver::iterator iEndpoint);

      void OnConnect(
        const asio::error_code& Error,
        asio::ip::tcp::resolver::iterator iEndpoint);

      void OnTimeout(const asio::error_code& Error);

    private:

      asio::io_service mIoService;

      asio::io_service mCallbackService;

      asio::ip::tcp::resolver mResolver;

      std::shared_ptr<dl::tcp::Session> mpSession;

      asio::basic_waitable_timer<std::chrono::system_clock> mTimer;

      std::string mHostname;

      unsigned mPort;

      std::mutex mConnectionMutex;

      std::vector<std::thread> mThreads;

      std::atomic<bool> mIsRunning;

      std::atomic<bool> mIsConnecting;

      std::unique_ptr<asio::io_service::work> mpNullIoWork;

      std::unique_ptr<asio::io_service::work> mpNullCallbackWork;

      dl::Signal<std::shared_ptr<SessionType>&> mSignalConnection;

      dl::Signal<const std::string> mSignalConnectionError;

      dl::Signal<const std::string> mSignalOnRx;

      dl::Signal<void> mSignalOnDisconnect;
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename SessionType>
  dl::tcp::Client<SessionType>::Client(
    const dl::tcp::ClientSettings<SessionType>& ClientSettings)
  : mIoService(),
    mCallbackService(),
    mResolver(mIoService),
    mpSession(nullptr),
    mTimer(mIoService),
    mHostname(ClientSettings.mHostname),
    mPort(ClientSettings.mPort),
    mConnectionMutex(),
    mThreads(),
    mIsRunning(true),
    mpNullIoWork(std::make_unique<asio::io_service::work> (mIoService)),
    mpNullCallbackWork(std::make_unique<asio::io_service::work>(mCallbackService))
  {
    if (ClientSettings.mConnectionCallback)
    {
      mSignalConnection.Connect(ClientSettings.mConnectionCallback);
    }

    if (ClientSettings.mConnectionErrorCallback)
    {
      mSignalConnectionError.Connect(ClientSettings.mConnectionErrorCallback);
    }

    if (ClientSettings.mOnRxCallback)
    {
      mSignalOnRx.Connect(ClientSettings.mOnRxCallback);
    }

    if (ClientSettings.mOnDisconnectCallback)
    {
      mSignalOnDisconnect.Connect(ClientSettings.mOnDisconnectCallback);
    }

    StartWorkerThreads(mCallbackService, ClientSettings.mNumberOfCallbackThreads);

    Connect();

    StartWorkerThreads(mIoService, ClientSettings.mNumberOfIoThreads);
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename SessionType>
  dl::tcp::Client<SessionType>::~Client()
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

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename SessionType>
  void dl::tcp::Client<SessionType>::StartWorkerThreads(
    asio::io_service& IoService,
    unsigned NumberOfThreads)
  {
    for (unsigned i = 0u; i < NumberOfThreads; ++i)
    {
      mThreads.emplace_back([&IoService] { IoService.run(); });
    }
  }
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename SessionType>
  void dl::tcp::Client<SessionType>::Connect()
  {
    mpSession =
      dl::tcp::SessionFactory::Create<SessionType>(mIoService, mCallbackService);

    mpSession->GetOnRxSignal().Connect(
      [this] (const std::string Bytes)
      {
        mSignalOnRx(Bytes);
      });

    mpSession->GetOnDisconnectSignal().Connect(
      [this] { StartConnect(); mSignalOnDisconnect();});

    StartConnect();
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename SessionType>
  void dl::tcp::Client<SessionType>::StartConnect()
  {
    asio::ip::tcp::resolver::query Query(mHostname, std::to_string(mPort));

    mResolver.async_resolve(
      Query,
      [this]
      (const asio::error_code& Error, asio::ip::tcp::resolver::iterator iEndpoint)
      {
        OnResolve(Error, iEndpoint);
      });
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename SessionType>
  void dl::tcp::Client<SessionType>::OnResolve(
    const asio::error_code& Error,
    asio::ip::tcp::resolver::iterator iEndpoint)
  {
    if (!Error)
    {
      mTimer.expires_from_now(std::chrono::seconds(2));

      asio::async_connect(
        mpSession->GetSocket(),
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

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename SessionType>
  void dl::tcp::Client<SessionType>::OnConnect(
    const asio::error_code& Error,
    asio::ip::tcp::resolver::iterator iEndpoint)
  {
    if (!Error)
    {
      mpSession->Start();

      mSignalConnection(mpSession);

      mTimer.cancel();
    }
    else if (iEndpoint != asio::ip::tcp::resolver::iterator())
    {
      asio::async_connect(
        mpSession->GetSocket(),
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

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename SessionType>
  void dl::tcp::Client<SessionType>::OnTimeout(const asio::error_code& Error)
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

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename SessionType>
  void dl::tcp::Client<SessionType>::Write(const std::string& Bytes)
  {
    if (mpSession)
    {
      mpSession->Write(Bytes);
    }
  }
}
