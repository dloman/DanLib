#include "Serial.hpp"
#include <experimental/filesystem>
#include <iostream>
#include <regex>
using dl::serial::Serial;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Serial::Serial(const std::string& Port, const unsigned Baudrate)
 : mIoService(),
   mCallbackService(),
   mSerialPort(mIoService),
   mpNullWork(std::make_shared<asio::io_service::work> (mCallbackService)),
   mWriteQueue(),
   mThreads(),
   mOnRxSignal(),
   mData(),
   mStrand(mIoService)
{
  mSerialPort.open(Port);

  SetOptions(Baudrate);

  mIoService.post([this] { ReadData(); });

  StartWorkerThreads(mIoService, 1);

  StartWorkerThreads(mCallbackService, 1);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Serial::~Serial()
{
  mIoService.stop();

  mpNullWork.reset();

  for (auto& Thread : mThreads)
  {
    Thread.join();
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Serial::StartWorkerThreads(
  asio::io_service& IoService,
  unsigned NumberOfThreads)
{
  for (unsigned i = 0u; i < NumberOfThreads; ++i)
  {
    mThreads.emplace_back([this, &IoService] { IoService.run();});
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Serial::SetOptions(const unsigned Baudrate)
{
  mSerialPort.set_option(asio::serial_port_base::baud_rate(Baudrate));

  mSerialPort.set_option(asio::serial_port_base::character_size(8));

  mSerialPort.set_option(
    asio::serial_port_base::stop_bits(
      asio::serial_port_base::stop_bits::one));

  mSerialPort.set_option(
    asio::serial_port_base::parity(
      asio::serial_port_base::parity::none));

  mSerialPort.set_option(
    asio::serial_port_base::flow_control(
      asio::serial_port_base::flow_control::none));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Serial::ReadData()
{
  mSerialPort.async_read_some(
    asio::buffer(mData.data(), mMaxLength),
    [this] (const asio::error_code& Error, size_t BytesReceived)
    {
      if (Error)
      {
        mOnReadErrorSignal(Error.message());
      }
      else
      {
        mCallbackService.post(
          [this, BytesReceived]
          {
            mOnRxSignal(std::string(mData.data(), BytesReceived));
          });
      }
      ReadData();
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Serial::Write(const std::string& Bytes)
{
  mIoService.post(mStrand.wrap(
    [this, Bytes = std::move(Bytes)]
    {
      mWriteQueue.push_back(std::move(Bytes));

      AsyncWrite();
    }));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Serial::AsyncWrite()
{
  if (!mWriteQueue.empty())
  {
    auto Message = mWriteQueue.front();

    mWriteQueue.pop_front();

    mSerialPort.async_write_some(
      asio::buffer(Message),
      mStrand.wrap(
        [this, Message] (const asio::error_code& Error, const size_t BytesTransfered)
        {
          if (!Error)
          {
            if (Message.size() > BytesTransfered)
            {
              mWriteQueue.push_front(Message.substr(BytesTransfered));
            }
            AsyncWrite();
          }
          else
          {
            mCallbackService.post(
              [=]
              {
                mWriteErrorSignal(Error.message(), Message);
              });
          }
        }));
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::vector<std::string> Serial::GetAvailableSerialPorts()
{
  std::vector<std::string> SerialPorts;
  for (auto& File : std::experimental::filesystem::directory_iterator("/dev"))
  {
    const auto& Filename = File.path().filename().string();
    if (
      std::regex_match(Filename, std::regex("ttyUSB[[:digit:]]")) ||
      std::regex_match(Filename, std::regex("ttyACM[[:digit:]]")))
    {
      SerialPorts.emplace_back("/dev/" + Filename);
    }
  }

  return SerialPorts;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::unique_ptr<Serial> Serial::GetSerialConnection(const unsigned Baudrate)
{
  for (const auto Port : GetAvailableSerialPorts())
  {
    try
    {
      return std::make_unique<Serial>(Port, Baudrate);
    }
    catch(...)
    {
    }
  }
  return nullptr;
}

