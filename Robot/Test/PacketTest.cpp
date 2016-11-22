#include <Robot/PacketEncoder.hpp>
#include <Robot/PacketDecoder.hpp>

#include <boost/hana/assert.hpp>
#include <boost/hana/members.hpp>
#include <boost/hana/equal.hpp>

#include <iostream>

dl::robot::PacketDecoder gPacketDecoder;

namespace hana = boost::hana;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
template<typename Type>
void TestPacket(Type Packet, dl::robot::packet::PacketType PacketType)
{
  auto EncodedPacket = dl::robot::PacketEncoder::Encode(Packet);

  dl::robot::packet::Header Header;

  Header.mPacketType = PacketType;

  Header.mVersion = dl::robot::packet::CurrentPacketVersion;

  Header.mPayloadSize = sizeof(Packet);

  gPacketDecoder.Decode(Header, std::experimental::string_view(EncodedPacket));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  dl::robot::packet::MotorCommand MotorCommand;

  MotorCommand.mMotor0 = 1;
  MotorCommand.mMotor1 = 2;
  MotorCommand.mMotor2 = 3;

  gPacketDecoder.GetMotorCommandSignal().Connect(
    [&MotorCommand] (const auto& DecodedMotorCommand)
    {
      BOOST_HANA_RUNTIME_CHECK(
        hana::members(DecodedMotorCommand) == hana::members(MotorCommand));
    });

  try
  {
    TestPacket(MotorCommand, dl::robot::packet::PacketType::eMotorCommand);
  }
  catch (std::exception& Exception)
  {
    std::cerr << Exception.what() << std::endl;
    return 1;
  }

  return 0;
}


