#pragma once
#include <Http/RequestData.hpp>
#include <mutex>
#include <ostream>
#include <string>
#include <vector>

namespace dl::http
{
  class Request
  {
    public:

      Request();

      ~Request() = default;

      Request(const Request& Other);

      Request& operator = (const Request& Rhs);

      bool AddBytes(const std::string& Bytes);

      dl::http::RequestType GetRequestType() const;

      void SetRequestType(const dl::http::RequestType RequestType);

      const std::string& GetUrl() const;

      const std::vector<std::string>& GetHeader() const;

      const std::string& GetBody() const;

    private:

      void ParseRequest();

      void ParseHeader();

      void ParseContentLength();

      bool ParseBody();

    private:

      std::string mBytes;

      dl::http::RequestType mRequestType;

      std::string mUrl;

      std::vector<std::string> mHeader;

      size_t mContentLength;

      std::string mBody;

      std::mutex mMutex;
  };
}
