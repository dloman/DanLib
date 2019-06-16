#include "SslSession.hpp"

#include <iostream>
#include <memory>

using dl::tcp::SslSession;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::shared_ptr<SslSession> SslSession::Create(
  asio::io_service& IoService,
  asio::io_service& CallbackService)
{
  return std::shared_ptr<SslSession>(new SslSession(IoService, CallbackService));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
SslSession::SslSession(asio::io_service& IoService, asio::io_service& CallbackService)
 : dl::tcp::Session(IoService, CallbackService),
   mContext(asio::ssl::context::sslv23),
   mStream(mSocket, mContext)
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SslSession::Start()
{
  mStream.set_verify_mode(asio::ssl::verify_none);

  mStream.handshake(asio::ssl::stream_base::client);

  mStream.async_read_some(
    asio::buffer(mData, mMaxLength),
    [this] (const asio::error_code& Error, const size_t BytesTransfered)
    {
      OnRead(Error, BytesTransfered);
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SslSession::AsyncWrite()
{
  if (!mWriteQueue.empty())
  {
    auto Message = mWriteQueue.front();

    mWriteQueue.pop_front();

    asio::async_write(
      mStream,
      asio::buffer(Message),
      asio::bind_executor(
        mWriteStrand,
        [this, pThis = shared_from_this(), Message]
        (const asio::error_code& Error, const size_t BytesTransfered)
        {
          if (!Error)
          {
            AsyncWrite();
          }
          else
          {
            asio::bind_executor(
              mCallbackStrand,
              [=]
              {
                mSignalWriteError(Error, Message);
              });
          }
        }));
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SslSession::OnRead(const asio::error_code& Error, const size_t BytesTransfered)
{
  if (!Error)
  {
    std::string Bytes(mData.data(), BytesTransfered);
    asio::bind_executor(
      mCallbackStrand,
      [this, pWeak = weak_from_this(), Bytes = std::move(Bytes)]
      {
        if (auto pThis = pWeak.lock())
        {
          mSignalOnRx(Bytes);
        }
      });

    mStream.async_read_some(
      asio::buffer(mData, mMaxLength),
      [this] (const asio::error_code& Error, const size_t BytesTransfered)
      {
        OnRead(Error, BytesTransfered);
      });
  }
  else if (
    (Error == asio::error::eof) || (Error == asio::error::connection_reset))
  {
    asio::bind_executor(
      mCallbackStrand,
      [this, pThis = shared_from_this()] { mSignalOnDisconnect(); });
  }
  else
  {
    asio::bind_executor(
      mCallbackStrand,
      [this, Error, pThis = shared_from_this()] { mSignalReadError(Error); });
  }
}
