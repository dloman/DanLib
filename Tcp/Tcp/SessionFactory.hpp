#pragma once

#include <Tcp/Session.hpp>
#include <Tcp/SslSession.hpp>

namespace dl::tcp::SessionFactory
{
  template <typename T>
  typename std::enable_if_t<std::is_same_v<T, dl::tcp::SslSession>, std::shared_ptr<dl::tcp::Session>>
    CreateImpl(
      asio::io_service& IoService,
      asio::io_service& CallbackService)
  {
    return dl::tcp::SslSession::Create(IoService, CallbackService);
  }

  template <typename T>
  typename std::enable_if_t<std::is_same_v<T, dl::tcp::Session>, std::shared_ptr<dl::tcp::Session>>
    CreateImpl(
      asio::io_service& IoService,
      asio::io_service& CallbackService)
  {
    return dl::tcp::Session::Create(IoService, CallbackService);
  }

  template <typename T>
    static std::shared_ptr<dl::tcp::Session> Create(
      asio::io_service& IoService,
      asio::io_service& CallbackService)
    {
      return dl::tcp::SessionFactory::CreateImpl<T>(IoService, CallbackService);
    }
}
