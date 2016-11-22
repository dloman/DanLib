#pragma once

#include <Robot/Packets.hpp>
#include <Signal/Signal.hpp>

#include <experimental/string_view>

namespace dl::robot
{
  class PacketDecoder
  {
    public:

      PacketDecoder();

      static dl::robot::packet::Header DecodeHeader(std::experimental::string_view Bytes);

      void Decode(
        const dl::robot::packet::Header& Header,
        std::experimental::string_view Bytes);

      const dl::Signal<const dl::robot::packet::MotorCommand&>& GetMotorCommandSignal() const;

    private:

      void DecodeMotorCommandPacket(std::experimental::string_view Bytes);

      template <typename Type>
      Type DecodePacket(std::experimental::string_view& Bytes);

      template <typename Type>
      static Type Decode(std::experimental::string_view& Bytes);

    private:

      dl::Signal<const dl::robot::packet::MotorCommand&> mMotorCommandSignal;
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  inline
    const dl::Signal<const dl::robot::packet::MotorCommand&>&
    dl::robot::PacketDecoder::GetMotorCommandSignal() const
  {
    return mMotorCommandSignal;
  }
}
