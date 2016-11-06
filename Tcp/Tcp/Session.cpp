#include "Session.hpp"

#include <iostream>
#include <memory>

using dl::tcp::Session;

std::atomic<unsigned long> Session::mCount(0ul);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::shared_ptr<Session> Session::Create(
  asio::io_service& IoService,
  asio::io_service& CallbackService)
{
  return std::shared_ptr<Session>(new Session(IoService, CallbackService));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Session::Session(asio::io_service& IoService, asio::io_service& CallbackService)
 : mSessionId(++mCount),
   mIoService(IoService),
   mCallbackService(CallbackService),
   mSocket(mIoService),
   mStrand(mIoService)
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Session::Start()
{
  mSocket.async_read_some(
    asio::buffer(mData, mMaxLength),
    [this] (const asio::error_code& Error, const size_t BytesTransfered)
    {
      OnRead(Error, BytesTransfered);
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Session::Write(const std::string& Bytes)
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
void Session::AsyncWrite()
{
  if (!mWriteQueue.empty())
  {
    auto Message = mWriteQueue.front();

    mWriteQueue.pop_front();

    asio::async_write(
      mSocket,
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
void Session::OnRead(const asio::error_code& Error, const size_t BytesTransfered)
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

    mSocket.async_read_some(
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
