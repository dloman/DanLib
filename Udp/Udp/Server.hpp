#pragma once

#include <Signal/Signal.hpp>

#include <asio/io_service.hpp>
#include <asio/ip/udp.hpp>
#include <thread>

namespace dl::udp
{
  class Session;

  class Server
  {
    public:

      Server(
        const unsigned Port,
        const unsigned NumberOfIoThreads = 1,
        const unsigned NumberOfCallbackThreads = 1);

      ~Server();

      Server(const Server& Other) = delete;

      Server& operator = (const Server& Rhs) = delete;

      const dl::Signal<const std::string&, const asio::ip::address&>& GetOnRxSignal() const;

      const dl::Signal<const std::string&, const asio::error_code&>& GetOnWriteErrorSignal() const;

      void Write(
        const std::string& Bytes,
        const asio::ip::address& IpAddress = asio::ip::address_v4::broadcast());

    private:

      void OnRead(
        const asio::error_code& Error,
        const size_t BytesTransfered,
        const dl::udp::Session& Session);

      void StartWorkerThreads(
        asio::io_service& IoService,
        unsigned NumberOfThreads);

      void OnWrite(
        const asio::error_code& Error,
        const size_t BytesTransfered,
        const std::string& Bytes);

    private:

      asio::io_service mIoService;

      asio::io_service mCallbackService;

      std::shared_ptr<asio::io_service::work> mpNullWork;

      asio::ip::udp::socket mSocket;

      std::vector<std::thread> mThreads;

      const unsigned mPort;

      dl::Signal<const std::string&, const asio::ip::address&> mSignalOnRx;

      dl::Signal<const std::string&, const asio::error_code&> mSignalWriteError;
  };
}
