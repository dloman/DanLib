#pragma once

#include <Signal/Signal.hpp>
#include <Tcp/Session.hpp>

#include <asio.hpp>
#include <asio/ssl.hpp>

namespace dl::tcp
{
  class SslSession : public dl::tcp::Session
  {
    public:

      static std::shared_ptr<SslSession> Create(
        asio::io_service& IoService,
        asio::io_service& CallbackService);

      ~SslSession() = default;

      SslSession(const SslSession& Other) = delete;

      SslSession& operator = (const SslSession& Rhs) = delete;

      void Start() override;

      asio::ssl::stream<asio::ip::tcp::socket&>& GetStream();

      const dl::Signal<const std::string&>& GetSignalOnHandshakeError() const;

    protected:

      SslSession(asio::io_service& IoService, asio::io_service& CallbackService);

    private:

      void OnRead(const asio::error_code& Error, const size_t BytesTransfered) override;

      void AsyncWrite() override;

      void OnHandshake(const asio::error_code& Error);

    private:

      static std::atomic<unsigned long> mCount;

      asio::ssl::context mContext;

      asio::ssl::stream<asio::ip::tcp::socket&> mStream;
  };
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
asio::ssl::stream<asio::ip::tcp::socket&>& dl::tcp::SslSession::GetStream()
{
  return mStream;
}
