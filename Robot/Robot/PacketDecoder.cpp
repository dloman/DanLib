#include "PacketDecoder.hpp"

#include <boost/hana/for_each.hpp>
#include <boost/hana/at_key.hpp>
#include <boost/hana/keys.hpp>

#include <cstring>

using dl::robot::PacketDecoder;
namespace hana = boost::hana;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
PacketDecoder::PacketDecoder()
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
dl::robot::packet::Header PacketDecoder::DecodeHeader(
  std::experimental::string_view Bytes)
{
  dl::robot::packet::Header Header;

  Header.mPacketType =
    static_cast<dl::robot::packet::PacketType>(Decode<uint8_t>(Bytes));

  Header.mVersion = Decode<uint8_t>(Bytes);

  Header.mPayloadSize = Decode<size_t>(Bytes);

  return Header;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
template<typename Type>
Type PacketDecoder::Decode(std::experimental::string_view& Bytes)
{
  Type Value;

  std::memcpy(&Value, Bytes.data(), sizeof(Type));

  Bytes.remove_prefix(sizeof(Type));

  return Value;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
template <typename Type>
Type PacketDecoder::DecodePacket(std::experimental::string_view& Bytes)
{
  Type Packet;

  constexpr auto Decoder =
    [](std::experimental::string_view& Bytes, auto& Object)
    {
      hana::for_each(hana::keys(Object), [&](auto&& Key)
      {
        auto& Member = hana::at_key(Object, Key);
        Member = Decode<std::remove_reference_t<decltype(Member)>> (Bytes);
      });
    };

  Decoder(Bytes, Packet);

  return Packet;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PacketDecoder::Decode(
  const dl::robot::packet::Header& Header,
  std::experimental::string_view Bytes)
{
  if (Header.mVersion != dl::robot::packet::CurrentPacketVersion)
  {
    throw std::logic_error(
      "ERROR: Invalid packet Version. Version " + std::to_string(Header.mVersion) +
      " does not equal current packet version " + std::to_string(dl::robot::packet::CurrentPacketVersion));
  }

  switch (Header.mPacketType)
  {
    case dl::robot::packet::PacketType::eMotorCommand:
      mMotorCommandSignal(DecodePacket<dl::robot::packet::MotorCommand> (Bytes));
      break;

    default:
      throw std::logic_error("ERROR: unknown packet type recieved");

  }
}
