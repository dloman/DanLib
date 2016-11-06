#pragma once

#include <Signal/Signal.hpp>

#include <asio/io_service.hpp>
#include <asio/serial_port.hpp>
#include <asio/strand.hpp>

#include <deque>
#include <thread>


namespace dl::serial
{
  class Serial
  {
    public:

      Serial(const std::string& Port, const unsigned Baudrate);

      Serial(const Serial& Other) = delete;

      Serial& operator = (const Serial& Rhs) = delete;

      const dl::Signal<const std::string&>& GetOnRxSignal() const;

      const dl::Signal<const std::string&>& GetOnReadErrorSignal() const;

      const dl::Signal<const std::string&>& GetConnectionErrorSignal() const;

      void Write(const std::string& Bytes);

    private:

      void StartWorkerThreads(
        asio::io_service& IoService,
        unsigned NumberOfThreads);

      void SetOptions(const unsigned Baudrate);

      void ReadData();

      void AsyncWrite();

    private:

      asio::io_service mIoService;

      asio::io_service mCallbackService;

      asio::serial_port mSerialPort;

      std::shared_ptr<asio::io_service::work> mpNullWork;

      std::deque<std::string> mWriteQueue;

      std::vector<std::thread> mThreads;

      dl::Signal<const std::string&> mOnRxSignal;

      dl::Signal<const std::string&> mOnReadErrorSignal;

      dl::Signal<const std::string&, const std::string&> mWriteErrorSignal;

      dl::Signal<const std::string&> mConnectionErrorSignal;

      static constexpr unsigned mMaxLength = 2048;

      std::array<char, mMaxLength> mData;

      asio::io_service::strand mStrand;
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  inline
  const dl::Signal<const std::string&>& dl::serial::Serial::GetOnRxSignal() const
  {
    return mOnRxSignal;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  inline
  const dl::Signal<const std::string&>& dl::serial::Serial::GetOnReadErrorSignal() const
  {
    return mOnReadErrorSignal;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  inline
  const dl::Signal<const std::string&>& dl::serial::Serial::GetConnectionErrorSignal() const
  {
    return mConnectionErrorSignal;
  }

  //----------------------------------------------------------------------------

}


