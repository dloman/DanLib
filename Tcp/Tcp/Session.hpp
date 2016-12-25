#pragma once

#include <Signal/Signal.hpp>

#include <asio.hpp>

#include <deque>
#include <string>
#include <iostream>

namespace dl::tcp
{
  class Session : public std::enable_shared_from_this<Session>
  {
    public:

      static std::shared_ptr<Session> Create(
        asio::io_service& IoService,
        asio::io_service& CallbackService);

      ~Session() = default;

      Session(const Session& Other) = delete;

      Session& operator = (const Session& Rhs) = delete;

      virtual void Start();

      asio::ip::tcp::socket& GetSocket();

      void Write(const std::string& Bytes);

      const dl::Signal<const std::string&>& GetOnRxSignal() const;

      const dl::Signal<void>& GetOnDisconnectSignal() const;

      const dl::Signal<const asio::error_code&>& GetSignalReadError() const;

      const dl::Signal<const asio::error_code&, const std::string&>& GetSignalWriteError() const;

      const unsigned long& GetSessionId() const;

    protected:

      Session(asio::io_service& IoService, asio::io_service& CallbackService);

      virtual void OnRead(const asio::error_code& Error, const size_t BytesTransfered);

      virtual void AsyncWrite();

      void OnWrite(const asio::error_code&, const size_t BytesTransfered);

    protected:

      static std::atomic<unsigned long> mCount;

      const unsigned long mSessionId;

      asio::io_service& mIoService;

      asio::io_service& mCallbackService;

      asio::ip::tcp::socket mSocket;

      std::deque<std::string> mWriteQueue;

      static constexpr unsigned mMaxLength = 1024;

      std::array<char, mMaxLength> mData;

      dl::Signal<const std::string&> mSignalOnRx;

      dl::Signal<void> mSignalOnDisconnect;

      dl::Signal<const asio::error_code&> mSignalReadError;

      dl::Signal<const asio::error_code&, const std::string&> mSignalWriteError;

      asio::io_service::strand mStrand;
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
const dl::Signal<const asio::error_code&>& dl::tcp::Session::GetSignalReadError() const
{
  return mSignalReadError;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<const asio::error_code&, const std::string&>&
  dl::tcp::Session::GetSignalWriteError() const
{
  return mSignalWriteError;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
asio::ip::tcp::socket& dl::tcp::Session::GetSocket()
{
  return mSocket;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const unsigned long& dl::tcp::Session::GetSessionId() const
{
  return mSessionId;
}
