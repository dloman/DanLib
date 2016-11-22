#pragma once

#include <boost/hana/for_each.hpp>
#include <boost/hana/fwd/members.hpp>

#include <sstream>

namespace dl::robot
{
  class PacketEncoder
  {
    public:

      template<typename Type>
      static std::string Encode(Type ThingToEncode)
      {
        std::ostringstream OutputStream;
        auto Serialize = [](std::ostream& OutputStream, auto const& Object)
        {
          boost::hana::for_each(
            boost::hana::members(Object),
            [&](auto Member) { OutputStream << Member; });
        };

        Serialize(OutputStream, ThingToEncode);

        return OutputStream.str();
      }

  };
}

