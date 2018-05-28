#include "Session.hpp"

#include <iostream>
#include <memory>

using dl::ws::Session;

std::atomic<unsigned long> Session::mCount(0ul);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::shared_ptr<Session> Session::Create(
  boost::asio::io_service& IoService,
  boost::asio::io_service& CallbackService)
{
  return std::shared_ptr<Session>(new Session(IoService, CallbackService));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Session::Session(
  boost::asio::io_service& IoService,
  boost::asio::io_service& CallbackService)
 : mSessionId(++mCount),
   mIoService(IoService),
   mCallbackService(CallbackService),
   mSocket(mIoService),
   mWebsocket(mSocket),
   mStrand(mIoService),
   mBuffer()
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Session::Start()
{
  mWebsocket.async_accept(
    [this] (const boost::system::error_code& Error)
    {
      OnAccept(Error);
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Session::OnAccept(const boost::system::error_code& Error)
{
  if (Error)
  {
    mSignalOnDisconnect();

    mSignalError(Error, "On Accept");

    return;
  }


  DoRead();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Session::DoRead()
{
  mWebsocket.async_read(
    mBuffer,
    [this] (const boost::system::error_code& Error, std::size_t BytesTransfered)
    {
      OnRead(Error, BytesTransfered);
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Session::OnRead(const boost::system::error_code& Error, const size_t BytesTransfered)
{
  if (!Error)
  {
    auto Bytes = boost::beast::buffers_to_string(mBuffer.data());

    mCallbackService.post(
      [this, pWeak = weak_from_this(), Bytes = std::move(Bytes)]
      {
        if (auto pThis = pWeak.lock())
        {
          mSignalOnRx(Bytes);
        }
      });

    mBuffer.consume(mBuffer.size());

    DoRead();
  }
  else if (
    Error == boost::beast::websocket::error::closed ||
    Error == boost::asio::error::eof ||
    Error == boost::asio::error::connection_reset)
  {
    mCallbackService.post(
      [this, pWeak = weak_from_this()]
    {
      if (auto pThis = pWeak.lock())
      {
        mSignalOnDisconnect();
      }
    });
  }
  else
  {
    mCallbackService.post(
      [this, Error, pThis = shared_from_this()]
      {
        mSignalError(Error, "Read Error");
      });
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Session::AsyncWrite()
{
  if (!mWriteQueue.empty())
  {
    DataType WriteDataType(DataType::eBinary);

    std::tie(mWriteBuffer, WriteDataType) = std::move(mWriteQueue.front());

    mWriteQueue.pop_front();

    mWebsocket.text(WriteDataType == DataType::eText);

    mWebsocket.async_write(
      boost::asio::buffer(mWriteBuffer),
      mStrand.wrap(
        [this, pThis = shared_from_this()]
        (const boost::system::error_code& Error, const size_t BytesTransfered)
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
                mSignalError(Error, "Write Error");
              });
          }
        }));
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Session::Write(const std::string& Bytes, dl::ws::DataType DataType)
{
  mIoService.post(mStrand.wrap(
    [this, DataType, Bytes = std::move(Bytes), pThis = shared_from_this()]
    {
      mWriteQueue.emplace_back(std::move(Bytes), DataType);

      AsyncWrite();
    }));
}
