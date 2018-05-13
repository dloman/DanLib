#pragma once

#include <Signal/Signal.hpp>

#include <asio.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <deque>
#include <string>
#include <iostream>

namespace dl::ws
{
  enum class DataType
  {
    eText,
    eBinary
  };

  class Session : public std::enable_shared_from_this<Session>
  {
    public:

      static std::shared_ptr<Session> Create(
        boost::asio::io_service& IoService,
        boost::asio::io_service& CallbackService);

      ~Session() = default;

      Session(const Session& Other) = delete;

      Session& operator = (const Session& Rhs) = delete;

      virtual void Start();

      void Write(const std::string& Bytes, dl::ws::DataType DataType);

      const dl::Signal<const std::string&>& GetOnRxSignal() const;

      const dl::Signal<void>& GetOnDisconnectSignal() const;

      const dl::Signal<const boost::system::error_code&, const std::string&>& GetSignalError() const;

      const unsigned long& GetSessionId() const;

      boost::asio::ip::tcp::socket& GetSocket();

    protected:

      Session(
        boost::asio::io_service& IoService,
        boost::asio::io_service& CallbackService);

      void OnAccept(const boost::system::error_code& Error);

      void DoRead();

      virtual void OnRead(const boost::system::error_code& Error, const size_t BytesTransfered);

      virtual void AsyncWrite();

      void OnWrite(const boost::system::error_code&, const size_t BytesTransfered);

    protected:

      static std::atomic<unsigned long> mCount;

      const unsigned long mSessionId;

      boost::asio::io_service& mIoService;

      boost::asio::io_service& mCallbackService;

      boost::asio::ip::tcp::socket mSocket;

      std::deque<std::pair<std::string, DataType>> mWriteQueue;

      std::string mWriteBuffer;

      static constexpr unsigned mMaxLength = 1024;

      dl::Signal<const std::string&> mSignalOnRx;

      dl::Signal<void> mSignalOnDisconnect;

      dl::Signal<const boost::system::error_code&, const std::string&> mSignalError;

      boost::beast::websocket::stream<boost::asio::ip::tcp::socket&> mWebsocket;

      boost::asio::io_service::strand mStrand;

      boost::beast::flat_buffer mBuffer;

  };
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
boost::asio::ip::tcp::socket& dl::ws::Session::GetSocket()
{
  return mSocket;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<const std::string&>& dl::ws::Session::GetOnRxSignal() const
{
  return mSignalOnRx;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<void>& dl::ws::Session::GetOnDisconnectSignal() const
{
  return mSignalOnDisconnect;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<const boost::system::error_code&, const std::string&>&
  dl::ws::Session::GetSignalError() const
{
  return mSignalError;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const unsigned long& dl::ws::Session::GetSessionId() const
{
  return mSessionId;
}
