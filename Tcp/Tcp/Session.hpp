#pragma once

#include <asio.hpp>

namespace dl::tcp
{
  class Session
  {
    public:

      Session(asio::io_service& Service);

      void Start();

      asio::ip::tcp::socket& GetSocket();

      void Write(const std::string& Bytes);

    private:

      void OnRead(const asio::error_code& Error, const size_t BytesTransfered);

    private:

      asio::ip::tcp::socket mSocket;

      enum { eMaxLength = 1024 };

      char mData[eMaxLength];
  };
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
asio::ip::tcp::socket& dl::tcp::Session::GetSocket()
{
  return mSocket;
}


