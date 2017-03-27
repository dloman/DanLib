#pragma once
#include <functional>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
namespace dl::http
{
  enum class RequestType : int
  {
    eGet = 0,
    ePost = 1,
    eFail = 2,
    eUnknown = 3
  };

  class RequestData
  {
    public:

      RequestType mType;

      std::string mUrl;

      bool operator==(const RequestData& Rhs) const
      {
        return (mType == Rhs.mType) && (mUrl == Rhs.mUrl);
      }

  };
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
namespace std
{
  template <>
    struct hash<dl::http::RequestData>
    {
      size_t operator()(const dl::http::RequestData& Arg) const
      {
        size_t result = 17;

        result = result * 31 +
          hash<std::uint8_t>()(static_cast<std::uint8_t>(Arg.mType));

        return result * 31 + hash<string>()(Arg.mUrl);
      }
    };
}
