#include "Session.hpp"

#include <iostream>

using dl::tcp::Session;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Session::Session(asio::io_service& IoService)
 : mSocket(IoService)
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Session::Start()
{
  mSocket.async_read_some(
    asio::buffer(mData, eMaxLength),
    [this] (const asio::error_code& Error, const size_t BytesTransfered)
    {
      OnRead(Error, BytesTransfered);
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Session::Write(const std::string& Bytes)
{
  asio::async_write(
    mSocket,
    asio::buffer(mData, eMaxLength),
    [this, &Bytes] (const asio::error_code& Error, const size_t BytesTransfered)
    {
      if (Error)
      {
        Write(Bytes);
      }
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Session::OnRead(const asio::error_code& Error, const size_t BytesTransfered)
{
  if (!Error)
  {

    mSignalOnRx(std::string(mData, BytesTransfered));

    mSocket.async_read_some(
      asio::buffer(mData, eMaxLength),
      [this] (const asio::error_code& Error, const size_t BytesTransfered)
      {
        OnRead(Error, BytesTransfered);
      });
  }
  else if (
    (Error == asio::error::eof) || (Error == asio::error::connection_reset))
  {
    mSignalOnDisconnect();
    std::cout << "disconnect" << std::endl;
  }
  else
  {
    std::cout << "nope" << std::endl;
  }
}
