#pragma once

#include <Robot/Packets.hpp>
#include <Signal/Signal.hpp>

#include <experimental/string_view>

namespace dl::robot
{
  class PacketAssembler
  {
    public:

      PacketAssembler() = default;

      void AddBytes(std::string&& Bytes);

      const dl::Signal<std::experimental::string_view>& GetPacketAssembledSignal() const;

    private:

      void AssemblePacket();

      void AssemblePacket(std::experimental::string_view Bytes);

    private:

      std::string mBytes;

      dl::Signal<std::experimental::string_view> mPacketAssembledSignal;

      static constexpr size_t mHeaderSize = sizeof(dl::robot::packet::Header);
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  inline const dl::Signal<std::experimental::string_view>&
    dl::robot::PacketAssembler::GetPacketAssembledSignal() const
  {
    return mPacketAssembledSignal;
  }
}
