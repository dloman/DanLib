#pragma once

#include <boost/hana/define_struct.hpp>

#include <cstdint>
#include <stddef.h>

namespace dl::robot::packet
{
  struct Header
  {
    //mPacketTypeIndex;
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

  constexpr uint8_t CurrentPacketVersion = 0;
}
