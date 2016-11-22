#include <Robot/PacketEncoder.hpp>
#include <Robot/PacketDecoder.hpp>
#include <Robot/Packets.hpp>

#include <boost/hana/assert.hpp>
#include <boost/hana/members.hpp>
#include <boost/hana/equal.hpp>

#include <iostream>

dl::robot::PacketDecoder<dl::robot::packet::MotorCommand> gPacketDecoder;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
template<typename Type>
void TestPacket(Type Packet)
{
  auto EncodedPacket = dl::robot::PacketEncoder::Encode(Packet);

  dl::robot::packet::Header Header;

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

  gPacketDecoder.GetSignal<dl::robot::packet::MotorCommand>().Connect(
    [&MotorCommand] (const auto& DecodedMotorCommand)
    {
    BOOST_HANA_RUNTIME_CHECK(
      boost::hana::members(DecodedMotorCommand) == boost::hana::members(MotorCommand));
    });

  try
  {
    TestPacket(MotorCommand);

    MotorCommand.mMotor0 = 69;

    TestPacket(MotorCommand);
  }
  catch (std::exception& Exception)
  {
    std::cerr << Exception.what() << std::endl;
    return 1;
  }

  return 0;
}


