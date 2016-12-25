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
  class Client
  {
    public:

      Client(
        const std::string& Hostname = "localhost",
        const unsigned Port = 8181,
        const unsigned NumberOfIoThreads = 2,
        const unsigned NumberOfCallbackThreads = 2);

      ~Client();

      Client(const Client& Other) = delete;

      Client& operator = (const Client& Rhs) = delete;

      void Write(const std::string& Bytes);

      const dl::Signal<const std::string>& GetOnRxSignal() const;

      const dl::Signal<void>& GetConnectionSignal() const;

      const dl::Signal<const std::string>& GetConnectionErrorSignal() const;

      const dl::Signal<void>& GetOnDisconnectSignal() const;

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

      dl::Signal<void> mSignalConnection;

      dl::Signal<const std::string> mSignalConnectionError;

      dl::Signal<const std::string> mSignalOnRx;

      dl::Signal<void> mSignalOnDisconnect;
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename SessionType>
  dl::tcp::Client<SessionType>::Client(
    const std::string& Hostname,
    const unsigned Port,
    const unsigned NumberOfIoThreads,
    const unsigned NumberOfCallbackThreads)
  : mIoService(),
    mCallbackService(),
    mResolver(mIoService),
    mpSession(nullptr),
    mTimer(mIoService),
    mHostname(Hostname),
    mPort(Port),
    mConnectionMutex(),
    mThreads(),
    mIsRunning(true),
    mpNullIoWork(std::make_unique<asio::io_service::work> (mIoService)),
    mpNullCallbackWork(std::make_unique<asio::io_service::work>(mCallbackService))
  {
    StartWorkerThreads(mCallbackService, NumberOfCallbackThreads);

    Connect();

    StartWorkerThreads(mIoService, NumberOfIoThreads);
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

      mSignalConnection();

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

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename SessionType>
  const dl::Signal<const std::string>& dl::tcp::Client<SessionType>::GetOnRxSignal() const
  {
    return mSignalOnRx;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename SessionType>
  const dl::Signal<void >& dl::tcp::Client<SessionType>::GetConnectionSignal() const
  {
    return mSignalConnection;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename SessionType>
  const dl::Signal<const std::string>& dl::tcp::Client<SessionType>::GetConnectionErrorSignal() const
  {
    return mSignalConnectionError;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename SessionType>
  const dl::Signal<void>& dl::tcp::Client<SessionType>::GetOnDisconnectSignal() const
  {
    return mSignalOnDisconnect;
  }
}
