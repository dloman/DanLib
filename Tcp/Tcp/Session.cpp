#include "Session.hpp"

#include <iostream>

using dl::tcp::Session;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Session::Session(asio::io_service& IoService, asio::io_service& CallbackService)
 : mIoService(IoService),
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
        [this, &Message] (const asio::error_code& Error, const size_t BytesTransfered)
        {
          if (!Error)
          {
            AsyncWrite();
          }
          else
          {
            CallSignalOnThreadPool(mSignalWriteError, Error, Message);
          }
        }));
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Session::Write(const std::string& Bytes)
{
  mIoService.post(mStrand.wrap(
    [this, Bytes] { mWriteQueue.push_back(Bytes); AsyncWrite(); }));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Session::OnRead(const asio::error_code& Error, const size_t BytesTransfered)
{
  if (!Error)
  {

    CallSignalOnThreadPool(mSignalOnRx, std::string(mData, BytesTransfered));

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
