#pragma once

#include <Signal/Signal.hpp>

#include <asio.hpp>

#include <string>

namespace dl::tcp
{
  class Session
  {
    public:

      Session(asio::io_service& Service);

      void Start();

      asio::ip::tcp::socket& GetSocket();

      void Write(const std::string& Bytes);

      const dl::Signal<const std::string&>& GetOnRxSignal() const;

      const dl::Signal<void>& GetOnDisconnectSignal() const;

    private:

      void OnRead(const asio::error_code& Error, const size_t BytesTransfered);

    private:

      asio::ip::tcp::socket mSocket;

      enum { eMaxLength = 1024 };

      char mData[eMaxLength];

      dl::Signal<const std::string&> mSignalOnRx;

      dl::Signal<void> mSignalOnDisconnect;
  };
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<const std::string&>& dl::tcp::Session::GetOnRxSignal() const
{
  return mSignalOnRx;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<void>& dl::tcp::Session::GetOnDisconnectSignal() const
{
  return mSignalOnDisconnect;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
asio::ip::tcp::socket& dl::tcp::Session::GetSocket()
{
  return mSocket;
}


