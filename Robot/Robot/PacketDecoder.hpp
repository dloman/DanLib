#pragma once

#include <Robot/Packets.hpp>
#include <Signal/Signal.hpp>

#include <boost/hana/for_each.hpp>
#include <boost/hana/at_key.hpp>
#include <boost/hana/keys.hpp>

#include <cstring>
#include <experimental/string_view>
#include <tuple>

namespace dl::robot
{
  template <typename ... PacketTypes>
  class PacketDecoder
  {
    public:

      PacketDecoder() = default;

      static dl::robot::packet::Header DecodeHeader(
        std::experimental::string_view Bytes)
      {
        dl::robot::packet::Header Header;

        Header.mVersion = Decode<uint8_t>(Bytes);

        Header.mPayloadSize = Decode<size_t>(Bytes);

        return Header;
      }

      void Decode(
        const dl::robot::packet::Header& Header,
        std::experimental::string_view Bytes)
      {
        if (Header.mVersion != dl::robot::packet::CurrentPacketVersion)
        {
          throw std::logic_error(
            "ERROR: Invalid packet Version. Version " +
            std::to_string(Header.mVersion) +
            " does not equal current packet version " +
            std::to_string(dl::robot::packet::CurrentPacketVersion));
        }

        //figure out how to save type and switch on saved type
        {
          dl::robot::PacketDecoder<PacketTypes...>::DecodePacket<dl::robot::packet::MotorCommand>(Bytes);
        }
      }

      template <typename PacketType>
      const dl::Signal<const PacketType&>& GetSignal() const
      {
        return std::get<dl::Signal<const PacketType&>>(mPacketSignals);
      }

    private:

      template <typename Type>
      Type DecodePacket(std::experimental::string_view& Bytes)
      {
        Type Packet;

        constexpr auto Decoder =
          [](std::experimental::string_view& Bytes, auto& Object)
          {
            boost::hana::for_each(boost::hana::keys(Object), [&](auto&& Key)
              {
              auto& Member = boost::hana::at_key(Object, Key);
              Member = dl::robot::PacketDecoder<PacketTypes...>::Decode<std::remove_reference_t<decltype(Member)>> (Bytes);
              });
          };

        Decoder(Bytes, Packet);

        return Packet;
      }

      template <typename Type>
      static Type Decode(std::experimental::string_view& Bytes)
      {
        Type Value;

        std::memcpy(&Value, Bytes.data(), sizeof(Type));

        Bytes.remove_prefix(sizeof(Type));

        return Value;
      }

    private:

      std::tuple<dl::Signal<const PacketTypes&>...> mPacketSignals;
  };
}
