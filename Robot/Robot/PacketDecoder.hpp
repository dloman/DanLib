#pragma once

#include <Robot/Packets.hpp>
#include <Signal/Signal.hpp>

#include <boost/hana/for_each.hpp>
#include <boost/hana/at_key.hpp>
#include <boost/hana/keys.hpp>
#include <boost/type_index.hpp>
#include <boost/unordered_map.hpp>

#include <cstring>
#include <experimental/string_view>
#include <tuple>
#include <iostream>

namespace dl::robot
{
  template <typename ... PacketTypes>
  class PacketDecoder
  {
    public:

      PacketDecoder()
      {
        FillMap(mPacketTypes, mPacketDecoders);
      }

      static dl::robot::packet::Header DecodeHeader(
        std::string_view Bytes)
      {
        dl::robot::packet::Header Header;

        Header.mPacketTypeIndex = Decode<boost::typeindex::type_index>(Bytes);

        Header.mVersion = Decode<uint8_t>(Bytes);

        Header.mPayloadSize = Decode<size_t>(Bytes);

        return Header;
      }

      void Decode(
        const dl::robot::packet::Header& Header,
        std::string_view Bytes)
      {
        if (Header.mVersion != dl::robot::packet::CurrentPacketVersion)
        {
          throw std::logic_error(
            "ERROR: Invalid packet Version. Version " +
            std::to_string(Header.mVersion) +
            " does not equal current packet version " +
            std::to_string(dl::robot::packet::CurrentPacketVersion));
        }

        if (mPacketDecoders.count(Header.mPacketTypeIndex))
        {
          mPacketDecoders[Header.mPacketTypeIndex](Bytes);
        }
        else
        {
          std::cerr << "WHY MUST I CRY" << std::endl;
        }
      }

      template <typename PacketType>
      const dl::Signal<const PacketType&>& GetSignal() const
      {
        return std::get<dl::Signal<const PacketType&>>(mPacketSignals);
      }

      template <typename Type>
      Type DecodePacket(std::string_view& Bytes) const
      {
        Type Packet;

        constexpr auto Decoder =
          [](std::string_view& Bytes, auto& Object)
          {
            boost::hana::for_each(boost::hana::keys(Object), [&](auto&& Key)
              {
                auto& Member = boost::hana::at_key(Object, Key);
                //Member =
                //dl::robot::PacketDecoder<PacketTypes...>::Decode<
                //std::remove_reference_t<decltype(Member)>> (Bytes);

              });
          };

        Decoder(Bytes, Packet);

        return Packet;
      }

    private:

      template <typename Type>
      static Type Decode(std::string_view& Bytes)
      {
        Type Value;

        std::memcpy(&Value, Bytes.data(), sizeof(Type));

        Bytes.remove_prefix(sizeof(Type));

        return Value;
      }

      template<std::size_t Index = 0, typename TupleType, typename MapType>
        typename std::enable_if_t<Index != std::tuple_size<TupleType>::value>
        FillMap(const TupleType& Tuple, MapType& Map)
        {
          using PacketType = decltype(std::get<Index>(Tuple));
          Map[boost::typeindex::type_id<PacketType>()] =
            [&] (std::string_view& Bytes)
            {
              auto Packet = DecodePacket<std::decay_t<PacketType>>(Bytes);
              auto& Signal = std::get<dl::Signal<const PacketType&>>(mPacketSignals);
              Signal(Packet);
            };

          FillMap<Index + 1>(Tuple, Map);
        }

      template<std::size_t Index = 0, typename TupleType, typename MapType>
        typename std::enable_if_t<Index == std::tuple_size<TupleType>::value>
        FillMap(const TupleType& Tuple, MapType& Map)
        {
        }

    private:

      std::tuple<dl::Signal<const PacketTypes&>...> mPacketSignals;

      boost::unordered_map<
        boost::typeindex::type_index,
        std::function<void(std::string_view&)>> mPacketDecoders;

      std::tuple<PacketTypes...> mPacketTypes;
  };
}
