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

namespace dl::FillMap
{
  template<size_t I>
    struct FillMapImpl
    {
      template<typename PacketTuple, typename SignalTuple, typename MapType, typename DecoderType>
        static void FillMap(
          PacketTuple& Packets,
          SignalTuple& Signals,
          MapType& Map,
          DecoderType& Decoder,
          size_t Index)
        {
          if (Index == I - 1)
          {
            using PacketType = decltype(std::get<I - 1>(Packets));
            auto& Signal = std::get<I - 1>(Signals);
            Map[boost::typeindex::type_id<PacketType>()] =
              [&Signal, &Decoder] (std::experimental::string_view& Bytes)
              {
                Signal(Decoder.DecodePacket<PacketType>(Bytes));
              };
          }
          else
          {
            FillMapImpl<I - 1>::FillMap(Packets, Signals, Map, Decoder, Index);
          }
        }
    };

  template<>
    struct FillMapImpl<0>
    {
      template<typename PacketTuple, typename SignalTuple, typename MapType, typename DecoderType>
        static void FillMap(
          PacketTuple& Packets,
          SignalTuple& Signals,
          MapType& Map,
          DecoderType& Decoder,
          size_t Index)
        {
          assert(false);
        }
    };
}

namespace dl::robot
{
  template <typename ... PacketTypes>
  class PacketDecoder
  {
    public:

      PacketDecoder()
      {
        for(auto i = 0u; i < sizeof...(PacketTypes); ++i)
        {
          dl::FillMap::FillMapImpl<sizeof...(PacketTypes)>::FillMap(
            mPacketTypes,
            mPacketSignals,
            mPacketDecoders,
            *this,
            i);
        }
      }

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

        //mPacketDecoders[Header.mPacketTypeIndex](Bytes);
      }

      template <typename PacketType>
      const dl::Signal<const PacketType&>& GetSignal() const
      {
        return std::get<dl::Signal<const PacketType&>>(mPacketSignals);
      }

      template <typename Type>
      Type DecodePacket(std::experimental::string_view& Bytes) const
      {
        Type Packet;

        constexpr auto Decoder =
          [](std::experimental::string_view& Bytes, auto& Object)
          {
            boost::hana::for_each(boost::hana::keys(Object), [&](auto&& Key)
              {
                auto& Member = boost::hana::at_key(Object, Key);
                Member =
                  dl::robot::PacketDecoder<PacketTypes...>::Decode<
                    std::remove_reference_t<decltype(Member)>> (Bytes);
              });
          };

        Decoder(Bytes, Packet);

        return Packet;
      }

    private:

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

      boost::unordered_map<
        boost::typeindex::type_index,
        std::function<void(std::experimental::string_view&)>> mPacketDecoders;

      std::tuple<PacketTypes...> mPacketTypes;
  };
}
