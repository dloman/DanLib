#include "Server.hpp"
#include <iostream>
#include <Udp/Session.hpp>

using dl::udp::Server;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Server::Server(
  const unsigned Port,
  const unsigned NumberOfIoThreads,
  const unsigned NumberOfCallbackThreads)
  : mIoService(),
    mCallbackService(),
    mpNullWork(std::make_shared<asio::io_service::work> (mCallbackService)),
    mSocket(mIoService, asio::ip::udp::endpoint(asio::ip::udp::v4(), Port)),
    mThreads(),
    mPort(Port)
{
  auto pSession = std::make_shared<dl::udp::Session>(mPort);

  mSocket.async_receive_from(
    asio::buffer(pSession->GetData(), dl::udp::Session::mMaxLength),
    pSession->GetEndpoint(),
    [this, pSession] (const asio::error_code& Error, const size_t BytesTransfered)
    {
      OnRead(Error, BytesTransfered, *pSession);
    });

  StartWorkerThreads(mIoService, NumberOfIoThreads);

  StartWorkerThreads(mCallbackService, NumberOfCallbackThreads);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Server::~Server()
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
void Server::StartWorkerThreads(
  asio::io_service& IoService,
  unsigned NumberOfThreads)
{
  for (unsigned i = 0u; i < NumberOfThreads; ++i)
  {
    mThreads.emplace_back([this, &IoService] { IoService.run(); });
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Server::OnRead(
  const asio::error_code& Error,
  const size_t BytesTransfered,
  const dl::udp::Session& Session)
{
  if (!Error && BytesTransfered > 0)
  {
    std::string Bytes(Session.GetData(), BytesTransfered);

    mCallbackService.post(
      [this, Bytes = std::move(Bytes), IpAddress = Session.GetEndpoint().address()]
      {
        mSignalOnRx(Bytes, IpAddress);
      });
  }

  auto pSession = std::make_shared<dl::udp::Session>(mPort);

  mSocket.async_receive_from(
    asio::buffer(pSession->GetData(), dl::udp::Session::mMaxLength),
    pSession->GetEndpoint(),
    [this, pSession] (const asio::error_code& Error, const size_t BytesTransfered)
    {
      OnRead(Error, BytesTransfered, *pSession);
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Server::Write(
  const std::string& Bytes,
  const asio::ip::address& IpAddress)
{
  auto pEndpoint =
    std::make_shared<asio::ip::udp::endpoint>(IpAddress, mPort);

  mSocket.async_send_to(
    asio::buffer(Bytes.data(),Bytes.size()),
    *pEndpoint,
    [this, Bytes, pEndpoint]
    (const asio::error_code& Error, const size_t BytesTransfered)
    {
      OnWrite(Error, BytesTransfered, Bytes);
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Server::OnWrite(
  const asio::error_code& Error,
  const size_t BytesTransfered,
  const std::string& Bytes)
{
  if (Error)
  {
    mCallbackService.post(
      [this, Error, Bytes = std::move(Bytes)]
      {
        mSignalWriteError(Bytes, Error);
      });
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const dl::Signal<const std::string&, const asio::ip::address&>& Server::GetOnRxSignal() const
{
  return mSignalOnRx;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const dl::Signal<const std::string&, const asio::error_code&>& Server::GetOnWriteErrorSignal() const
{
  return mSignalWriteError;
}
