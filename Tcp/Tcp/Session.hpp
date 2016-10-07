#pragma once

#include <Signal/Signal.hpp>

#include <asio.hpp>

#include <deque>
#include <string>
#include <iostream>

namespace dl::tcp
{
  class Session
  {
    public:

      Session(asio::io_service& IoService, asio::io_service& CallbackService);

      ~Session() = default;

      Session(const Session& Other) = delete;

      Session& operator = (const Session& Rhs) = delete;

      void Start();

      asio::ip::tcp::socket& GetSocket();

      void Write(const std::string& Bytes);

      const dl::Signal<const std::string>& GetOnRxSignal() const;

      const dl::Signal<const unsigned long>& GetOnDisconnectSignal() const;

      const dl::Signal<const asio::error_code>& GetSignalReadError() const;

      const dl::Signal<const asio::error_code, const std::string>& GetSignalWriteError() const;

      const unsigned long& GetSessionId() const;

    private:

      void OnRead(const asio::error_code& Error, const size_t BytesTransfered);

      void AsyncWrite();

      template <typename ... T, typename ... ArgsType>
      void CallSignalOnThreadPool(dl::Signal<T...>& Signal, ArgsType&& ... Args);

    private:

      static std::atomic<unsigned long> mCount;

      const unsigned long mSessionId;

      asio::io_service& mIoService;

      asio::io_service& mCallbackService;

      asio::ip::tcp::socket mSocket;

      asio::io_service::strand mStrand;

      std::deque<std::string> mWriteQueue;

      static constexpr unsigned mMaxLength = 1024;

      char mData[mMaxLength];

      dl::Signal<const std::string> mSignalOnRx;

      dl::Signal<const unsigned long> mSignalOnDisconnect;

      dl::Signal<const asio::error_code> mSignalReadError;

      dl::Signal<const asio::error_code, const std::string> mSignalWriteError;
  };
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<const std::string>& dl::tcp::Session::GetOnRxSignal() const
{
  return mSignalOnRx;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<const unsigned long>& dl::tcp::Session::GetOnDisconnectSignal() const
{
  return mSignalOnDisconnect;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<const asio::error_code>& dl::tcp::Session::GetSignalReadError() const
{
  return mSignalReadError;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const dl::Signal<const asio::error_code, const std::string>&
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
template <typename ... T, typename ... ArgsType>
void dl::tcp::Session::CallSignalOnThreadPool(
  dl::Signal<T...>& Signal,
  ArgsType&& ... Args)
{
  mCallbackService.post(
    [&Signal, &Args...] { Signal(std::forward<ArgsType>(Args)...); });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
inline
const unsigned long& dl::tcp::Session::GetSessionId() const
{
  return mSessionId;
}
