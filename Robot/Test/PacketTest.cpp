#include <Robot/PacketEncoder.hpp>
#include <Robot/PacketDecoder.hpp>
#include <Robot/Packets.hpp>

#include <boost/hana/assert.hpp>
#include <boost/hana/members.hpp>
#include <boost/hana/equal.hpp>

#include <iostream>

using namespace dl::robot;
using DecoderType = PacketDecoder<packet::MotorCommand, packet::Position>;
DecoderType gPacketDecoder;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
template<typename Type>
void TestPacket(Type Packet)
{
  auto EncodedPacket = dl::robot::PacketEncoder::Encode(Packet);

  dl::robot::packet::Header Header;

  Header.mPacketTypeIndex = boost::typeindex::type_id<Type>();

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

  dl::robot::packet::Position Position;

  Position.mPostionX = 69;
  Position.mPostionY = 420;
  Position.mPostionZ = 1337;

  gPacketDecoder.GetSignal<dl::robot::packet::MotorCommand>().Connect(
    [&MotorCommand] (const auto& DecodedMotorCommand)
    {
      BOOST_HANA_RUNTIME_CHECK(
        boost::hana::members(DecodedMotorCommand) == boost::hana::members(MotorCommand));

      std::cout << "Motor Command = "
        << static_cast<int>(DecodedMotorCommand.mMotor0) << ','
        << static_cast<int>(DecodedMotorCommand.mMotor1) << ','
        << static_cast<int>(DecodedMotorCommand.mMotor2) << std::endl;
    });

  gPacketDecoder.GetSignal<dl::robot::packet::Position>().Connect(
    [&Position] (const auto& DecodedPosition)
    {
      BOOST_HANA_RUNTIME_CHECK(
        boost::hana::members(DecodedPosition) == boost::hana::members(Position));

      std::cout << "Position = "
        << DecodedPosition.mPostionX << ','
        << DecodedPosition.mPostionY << ','
        << DecodedPosition.mPostionZ << std::endl;
    });

  try
  {
    TestPacket(MotorCommand);

    MotorCommand.mMotor0 = 69;

    TestPacket(MotorCommand);

    TestPacket(Position);

    Position.mPostionX = 4444;

    TestPacket(Position);
  }
  catch (std::exception& Exception)
  {
    std::cerr << Exception.what() << std::endl;
    return 1;
  }

  return 0;
}
