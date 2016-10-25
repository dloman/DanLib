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
        [this, pThis, &Message] (const asio::error_code& Error, const size_t BytesTransfered)
        {
          if (!Error)
          {
            AsyncWrite(pThis);
          }
          else
          {
            CallSignalOnThreadPool(mSignalWriteError, Error, std::move(Message));
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
    CallSignalOnThreadPool(mSignalOnRx, std::move(Bytes));

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
    CallSignalOnThreadPool(mSignalOnDisconnect);
  }
  else
  {
    CallSignalOnThreadPool(mSignalReadError, Error);
  }
}
