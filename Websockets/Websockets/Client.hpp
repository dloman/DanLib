//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Dan Loman
// 2016-10-8
//
// Description:
//   This is a ws client
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#pragma once
#include <Signal/Signal.hpp>


#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <deque>
#include <memory>
#include <thread>
#include <variant>

namespace dl::ws
{
  enum class DataType;

  class Client
  {
    public:

      Client(
        const std::string& Hostname = "localhost",
        const unsigned Port = 8181,
        const unsigned NumberOfIoThreads = 1,
        const unsigned NumberOfCallbackThreads = 1);

      Client(
        boost::asio::ssl::context& Context,
        const std::string& Hostname = "localhost",
        const unsigned Port = 8181,
        const unsigned NumberOfIoThreads = 1,
        const unsigned NumberOfCallbackThreads = 1);
      virtual ~Client();

      Client(const Client& Other) = delete;

      Client& operator = (const Client& Rhs) = delete;

      std::shared_ptr<Client> getThis();

      void Connect();

      void Write(const std::string& Bytes, dl::ws::DataType DataType);

      void Write(std::string&& Bytes, dl::ws::DataType DataType);

      const dl::Signal<const std::string>& GetOnRxSignal() const;

      const dl::Signal<void>& GetConnectionSignal() const;

      const dl::Signal<const std::string>& GetErrorSignal() const;

      const dl::Signal<void>& GetOnDisconnectSignal() const;

    private:

      void StartWorkerThreads(
        boost::asio::io_service& IoService,
        unsigned NumberOfThreads);

      void Entry();

      void OnResolve(
        const boost::system::error_code& Error,
        boost::asio::ip::tcp::resolver::results_type Results);

      void OnConnect(
        const boost::system::error_code& Error,
        boost::asio::ip::tcp::resolver::iterator iEndpoint);

      void OnHandshake(const boost::system::error_code& Error);

      void OnSslHandshake(const boost::system::error_code& Error);

      void OnTimeout(const boost::system::error_code& Error);

      void DoRead();

			void OnRead(
				const boost::system::error_code& Error,
				const size_t BytesTransfered);

      void AsyncWrite();

    private:

      std::vector<std::thread> mThreads;

      boost::asio::io_service mIoService;

      boost::asio::io_service mCallbackService;

      boost::asio::ip::tcp::resolver mResolver;

      std::variant<
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket>,
        boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>> mWebsocket;

      boost::asio::basic_waitable_timer<std::chrono::system_clock> mTimer;

      std::string mHostname;

      unsigned mPort;

      std::unique_ptr<boost::asio::io_service::work> mpNullIoWork;

      std::unique_ptr<boost::asio::io_service::work> mpNullCallbackWork;

      boost::beast::flat_buffer mBuffer;

      std::deque<std::pair<std::string, dl::ws::DataType>> mWriteQueue;

      std::string mWriteBuffer;

      boost::asio::strand<boost::asio::io_context::executor_type> mStrand;

      dl::Signal<void> mSignalConnection;

      dl::Signal<const std::string> mSignalError;

      dl::Signal<const std::string> mSignalOnRx;

      dl::Signal<void> mSignalOnDisconnect;

      std::mutex mMutex;

      bool mIsSending;
  };
}

