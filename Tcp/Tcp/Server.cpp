//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Dan Loman
// 2016-09-14
//
// Description:
//   This is a tcp server
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "Server.hpp"
#include <iostream>

using dl::tcp::Server;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Server::Server(unsigned short Port)
  : mIoService(),
    mAcceptor(mIoService, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), Port)),
    mThread()
{
  mAcceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));

  StartAccept();

  mThread = std::thread([this] { mIoService.run(); });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Server::~Server()
{
  mIoService.stop();

  mThread.join();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Server::StartAccept()
{
  auto pSession = std::make_shared<dl::tcp::Session>(mIoService);

  mAcceptor.async_accept(
    pSession->GetSocket(),
    [this, pSession] (asio::error_code Error)
    {
      OnAccept(pSession, Error);
    });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Server::OnAccept(std::shared_ptr<dl::tcp::Session> pSession, asio::error_code& Error)
{
  if (!Error)
  {
    std::cout << "ACCEPT!" << std::endl;
    pSession->Start();
    mSignalNewSession(pSession);
  }
  else
  {
    std::cerr << "ERROR" << std::endl;
  }

  StartAccept();
}
