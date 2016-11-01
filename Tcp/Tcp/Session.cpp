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
  std::weak_ptr<Session> pWeak = weak_from_this();
  mIoService.post(mStrand.wrap(
    [this, Bytes, pWeak]
    {
      if (auto pThis = pWeak.lock())
      {
        mWriteQueue.push_back(Bytes);

        AsyncWrite(pThis);
      }
    }));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Session::AsyncWrite(std::weak_ptr<Session> pWeak)
{
  auto pThis = pWeak.lock();

  if (pThis && !mWriteQueue.empty())
  {
    auto Message = mWriteQueue.front();

    mWriteQueue.pop_front();

    asio::async_write(
      mSocket,
      asio::buffer(Message),
      mStrand.wrap(
        [this, pThis, Message] (const asio::error_code& Error, const size_t BytesTransfered)
        {
          if (!Error)
          {
            AsyncWrite(pThis);
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
    std::string Bytes(mData, BytesTransfered);
    mCallbackService.post(
      [this, pWeak = weak_from_this(), Bytes]
      {
        auto pThis = pWeak.lock();

        mSignalOnRx(Bytes);
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
    mCallbackService.post([this] { mSignalOnDisconnect(); });
  }
  else
  {
    mCallbackService.post([this, Error] { mSignalReadError(Error); });
  }
}
