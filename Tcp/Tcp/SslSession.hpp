#pragma once

#include <Signal/Signal.hpp>

#include <asio.hpp>
#include <asio/ssl.hpp>

#include <deque>
#include <string>
#include <iostream>

namespace dl::tcp
{
  class SslSession : public std::enable_shared_from_this<SslSession>
  {
    public:

      static std::shared_ptr<SslSession> Create(
        asio::io_service& IoService,
        asio::io_service& CallbackService);

      ~SslSession() = default;

      SslSession(const SslSession& Other) = delete;

      SslSession& operator = (const SslSession& Rhs) = delete;

      void Start();

      asio::ip::tcp::socket& GetSocket();

      asio::ssl::stream<asio::ip::tcp::socket&>& GetStream();

      void Write(const std::string& Bytes);

      const dl::Signal<const std::string&>& GetOnRxSignal() const;

      const dl::Signal<void>& GetOnDisconnectSignal() const;

      const dl::Signal<const asio::error_code&>& GetSignalReadError() const;

      const dl::Signal<const std::string&>& GetSignalOnHandshakeError() const;

      const dl::Signal<const asio::error_code&, const std::string&>& GetSignalWriteError() const;

      const unsigned long& GetSslSessionId() const;

    protected:

      SslSession(asio::io_service& IoService, asio::io_service& CallbackService);

    private:

      void OnRead(const asio::error_code& Error, const size_t BytesTransfered);

      void AsyncWrite();

      void OnWrite(const asio::error_code&, const size_t BytesTransfered);

      void OnHandshake(const asio::error_code& Error);

    private:

      static std::atomic<unsigned long> mCount;

      const unsigned long mSslSessionId;

      asio::io_service& mIoService;

      asio::io_service& mCallbackService;

      asio::ssl::context mContext;

      asio::ip::tcp::socket mSocket;

      asio::ssl::stream<asio::ip::tcp::socket&> mStream;

      std::deque<std::string> mWriteQueue;

      static constexpr unsigned mMaxLength = 1024;

      std::array<char, mMaxLength> mData;

      dl::Signal<const std::string&> mSignalOnRx;

      dl::Signal<const std::string&> mSignalOnHandshakeError;

      dl::Signal<void> mSignalOnDisconnect;

      dl::Signal<const asio::error_code&> mSignalReadError;

      dl::Signal<const asio::error_code&, const std::string&> mSignalWriteError;

      asio::io_service::strand mStrand;
  };
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<const std::string&>& dl::tcp::SslSession::GetOnRxSignal() const
{
  return mSignalOnRx;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<void>& dl::tcp::SslSession::GetOnDisconnectSignal() const
{
  return mSignalOnDisconnect;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<const asio::error_code&>& dl::tcp::SslSession::GetSignalReadError() const
{
  return mSignalReadError;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<const asio::error_code&, const std::string&>&
  dl::tcp::SslSession::GetSignalWriteError() const
{
  return mSignalWriteError;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
asio::ip::tcp::socket& dl::tcp::SslSession::GetSocket()
{
  return mSocket;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
asio::ssl::stream<asio::ip::tcp::socket&>& dl::tcp::SslSession::GetStream()
{
  return mStream;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const unsigned long& dl::tcp::SslSession::GetSslSessionId() const
{
  return mSslSessionId;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<const std::string&>& dl::tcp::SslSession::GetSignalOnHandshakeError() const
{
  return mSignalOnHandshakeError;
}
