#pragma once

#include <asio/ip/udp.hpp>

namespace dl::udp
{
  class Session
  {
    public:

      Session(
        const unsigned Port,
        const asio::ip::address_v4& IpAddress = asio::ip::address_v4::any());

      asio::ip::udp::endpoint& GetEndpoint();

      const asio::ip::udp::endpoint& GetEndpoint() const;

      const char* GetData() const;

      char* GetData();

      static constexpr unsigned mMaxLength = 1024;

    private:

      asio::ip::udp::endpoint mEndpoint;

      std::array<char, mMaxLength> mData;
  };
}
