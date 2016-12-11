#pragma once

#include <boost/hana/define_struct.hpp>

#include <cstdint>
#include <stddef.h>
#include <boost/type_index.hpp>

namespace dl::robot::packet
{
  struct Header
  {
    boost::typeindex::type_index mPacketTypeIndex;
    uint8_t mVersion;
    size_t mPayloadSize;
  };

  struct MotorCommand
  {
    BOOST_HANA_DEFINE_STRUCT(
      MotorCommand,
      (uint8_t, mMotor0),
      (uint8_t, mMotor1),
      (uint8_t, mMotor2)
      );
  };

  struct Position
  {
    BOOST_HANA_DEFINE_STRUCT(
      Position,
      (double, mPostionX),
      (double, mPostionY),
      (double, mPostionZ)
      );
  };

  constexpr uint8_t CurrentPacketVersion = 0;
}
