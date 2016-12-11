#include "SslSession.hpp"

#include <iostream>
#include <memory>

using dl::tcp::SslSession;

std::atomic<unsigned long> SslSession::mCount(0ul);

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
 : mSslSessionId(++mCount),
   mIoService(IoService),
   mCallbackService(CallbackService),
   mContext(asio::ssl::context::sslv23),
   mSocket(mIoService),
   mStream(mSocket, mContext),
   mStrand(mIoService)
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
void SslSession::Write(const std::string& Bytes)
{
  mIoService.post(mStrand.wrap(
    [this, Bytes = std::move(Bytes), pThis = shared_from_this()]
    {
      mWriteQueue.push_back(std::move(Bytes));

      AsyncWrite();
    }));
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
      mStrand.wrap(
        [this, pThis = shared_from_this(), Message]
        (const asio::error_code& Error, const size_t BytesTransfered)
        {
          if (!Error)
          {
            AsyncWrite();
          }
          else
          {
            mCallbackService.post(
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
    mCallbackService.post(
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
    mCallbackService.post(
      [this, pThis = shared_from_this()] { mSignalOnDisconnect(); });
  }
  else
  {
    mCallbackService.post(
      [this, Error, pThis = shared_from_this()] { mSignalReadError(Error); });
  }
}
