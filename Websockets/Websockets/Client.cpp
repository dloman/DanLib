#include "Client.hpp"
#include <Websockets/Session.hpp>

using dl::ws::Client;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
Client::Client(
  const std::string& Hostname,
  const unsigned Port,
  const unsigned NumberOfIoThreads,
  const unsigned NumberOfCallbackThreads)
: mThreads(),
  mIoService(),
  mCallbackService(),
  mResolver(mIoService),
  mWebsocket(mIoService),
  mTimer(mIoService),
  mHostname(Hostname),
  mPort(Port),
  mpNullIoWork(std::make_unique<boost::asio::io_service::work> (mIoService)),
  mpNullCallbackWork(std::make_unique<boost::asio::io_service::work>(mCallbackService)),
  mBuffer(),
  mStrand(mWebsocket.get_executor()),
  mIsSending(false)
{
  StartWorkerThreads(mCallbackService, NumberOfCallbackThreads);

  StartWorkerThreads(mIoService, NumberOfIoThreads);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
Client::~Client()
{
  mIoService.stop();

  mpNullIoWork.reset();

  mpNullCallbackWork.reset();

  for (auto& Thread : mThreads)
  {
    if (Thread.joinable())
    {
      Thread.join();
    }
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void Client::StartWorkerThreads(
  boost::asio::io_service& IoService,
  unsigned NumberOfThreads)
{
  for (unsigned i = 0u; i < NumberOfThreads; ++i)
  {
    mThreads.emplace_back([&IoService] { IoService.run(); });
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void Client::Connect()
{
  mResolver.async_resolve(
    mHostname,
    std::to_string(mPort),
    [this]
    (const boost::system::error_code& Error, boost::asio::ip::tcp::resolver::results_type Results)
    {
      OnResolve(Error, Results);
    });
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void Client::OnResolve(
  const boost::system::error_code& Error,
  boost::asio::ip::tcp::resolver::results_type Results)
{
  if (!Error)
  {
    mTimer.expires_from_now(std::chrono::seconds(2));

    boost::asio::async_connect(
      mWebsocket.next_layer(),
      Results.begin(),
      Results.end(),
      [this]
      (const boost::system::error_code& Error, boost::asio::ip::tcp::resolver::iterator iEndpoint)
      {
        OnConnect(Error, iEndpoint);
      });

    mTimer.async_wait([this] (const boost::system::error_code& Error) { OnTimeout(Error); });
  }
  else
  {
    mSignalError(Error.message());
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void Client::OnConnect(
  const boost::system::error_code& Error,
  boost::asio::ip::tcp::resolver::iterator iEndpoint)
{
  if (!Error)
  {
    mWebsocket.async_handshake(
      mHostname,
      "/",
      [this] (const boost::system::error_code& Error)
      {
        OnHandshake(Error);
      });

    mTimer.cancel();
  }
  else
  {
    mSignalError(Error.message());
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void Client::OnHandshake(const boost::system::error_code& Error)
{
  if (Error)
  {
    mSignalError(Error.message());
  }
  else
  {
    DoRead();

    mSignalConnection();
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void Client::OnTimeout(const boost::system::error_code& Error)
{
  if (!Error)
  {
    Connect();
  }
  else if (Error != boost::asio::error::operation_aborted)
  {
    mSignalError(Error.message());
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void Client::DoRead()
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
void Client::OnRead(
  const boost::system::error_code& Error,
  const size_t BytesTransfered)
{
  if (!Error)
  {
    auto Bytes = boost::beast::buffers_to_string(mBuffer.data());

    mCallbackService.post(
      [this, Bytes = std::move(Bytes)]
      {
        mSignalOnRx(Bytes);
      });

    DoRead();
  }
  else if (
    Error == boost::beast::websocket::error::closed ||
    Error == boost::asio::error::eof)
  {
    mCallbackService.post(
      [this]
    {
      mSignalOnDisconnect();

      Connect();
    });
  }
  else
  {
    mCallbackService.post(
      [this, Error]
      {
        mSignalError(Error.message());
      });
  }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void Client::AsyncWrite()
{
  std::lock_guard Lock(mMutex);

  if (!mWriteQueue.empty() && !mIsSending)
  {
    DataType WriteDataType(DataType::eBinary);

    std::tie(mWriteBuffer, WriteDataType) = std::move(mWriteQueue.front());

    mIsSending = true;

    mWriteQueue.pop_front();

    mWebsocket.text(WriteDataType == DataType::eText);

    mWebsocket.async_write(
      boost::asio::buffer(mWriteBuffer),
      boost::asio::bind_executor(
        mStrand,
        [this]
        (const boost::system::error_code& Error, const size_t BytesTransfered)
        {
          if (!Error)
          {
            {
              std::lock_guard Lock(mMutex);

              mIsSending = false;
            }

            AsyncWrite();
          }
          else
          {
            mCallbackService.post(
              [=]
              {
                mSignalError("Write Error " + Error.message());
              });
          }
        }));
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Client::Write(std::string&& Bytes, dl::ws::DataType DataType)
{
  mIoService.post(
    boost::asio::bind_executor(
      mStrand,
      [this, DataType, Bytes = std::move(Bytes)]
      {
        {
          std::lock_guard Lock(mMutex);

          mWriteQueue.emplace_back(std::move(Bytes), DataType);
        }

        AsyncWrite();
      }));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Client::Write(const std::string& Bytes, dl::ws::DataType DataType)
{
  mIoService.post(
    boost::asio::bind_executor(
      mStrand,
      [this, DataType, Bytes]
      {
        {
          std::lock_guard Lock(mMutex);

          mWriteQueue.emplace_back(std::move(Bytes), DataType);
        }

        AsyncWrite();
      }));
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
const dl::Signal<const std::string>& Client::GetOnRxSignal() const
{
  return mSignalOnRx;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
const dl::Signal<void >& Client::GetConnectionSignal() const
{
  return mSignalConnection;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
const dl::Signal<const std::string>& Client::GetErrorSignal() const
{
  return mSignalError;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
const dl::Signal<void>& Client::GetOnDisconnectSignal() const
{
  return mSignalOnDisconnect;
}
