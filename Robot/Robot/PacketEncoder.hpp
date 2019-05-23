#pragma once

#include <boost/hana/for_each.hpp>
#include <boost/hana/fwd/members.hpp>

#include <sstream>

namespace dl::robot
{
  template <typename T>
  inline
  void SerializeMember(std::ostream& Stream, const T& Member)
  {
    Stream.write(
      reinterpret_cast<char*>(const_cast<T*>(&Member)),
      sizeof(Member));
  }

  inline
  void SerializeMember(std::ostream& Stream, const std::string& String)
  {
    SerializeMember(Stream, String.size());

    Stream.write(String.data(), String.size());
  }

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
            [&](const auto& Member)
            {
              dl::robot::SerializeMember(OutputStream, Member);
            });
        };

        Serialize(OutputStream, ThingToEncode);

        return OutputStream.str();
      }

  };
}

