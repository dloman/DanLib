#pragma once
#include <mutex>
#include <ostream>
#include <string>
#include <vector>

namespace dl::http
{
  class Responce
  {
    public:

      enum class Status : int
      {
        eContinue = 100,
        eSwitchingProtocols = 101,
        eOk = 200,
        eCreated = 201,
        eAccepted = 202,
        eNonAuthoritativeInformation = 203,
        eNoContent = 204,
        eResetContent = 205,
        ePartialContent = 206,
        eMultipleChoices = 300,
        eMovedPermanently = 301,
        eFound = 302,
        eSeeOther = 303,
        eNotModified = 304,
        eUseProxy = 305,
        eTemporaryRedirect = 307,
        eBadRequest = 400,
        eUnauthorized = 401,
        ePaymentRequired = 402,
        eForbidden = 403,
        eNotFound = 404,
        eMethodNotAllowed = 405,
        eNotAcceptable = 406,
        eProxyAuthenticationRequired = 407,
        eRequestTimeOut = 408,
        eConflict = 409,
        eGone = 410,
        eLengthRequired = 411,
        ePreconditionFailed = 412,
        eRequestEntityTooLarge = 413,
        eRequestURITooLarge = 414,
        eUnsupportedMediaType = 415,
        eRequestedRangeNotSatisfiable = 416,
        eExpectationFailed = 417,
        eInternalServerError = 500,
        eNotImplemented = 501,
        eBadGateway = 502,
        eServiceUnavailable = 503,
        eGatewayTimeOut = 504,
        eHttpVersionNotSupported = 505,
        eParsingError = 0,
        eUninitialized = -1
      };

      static dl::http::Responce Error();

      static dl::http::Responce GenerateResponce(
        const Status status,
        const std::string& ContentType,
        const std::string& Body);

      Responce();

      ~Responce() = default;

      Responce(const Responce& Other);

      Responce& operator = (const Responce& Rhs);

      bool AddBytes(const std::string& Bytes);

      Status GetStatus() const;

      void SetStatus(const Status);

      const std::vector<std::string>& GetHeader() const;

      void SetHeader(const std::vector<std::string>&& Header);

      const std::string& GetBody() const;

      void SetBody(const std::string& Body);

      std::string ToBytes() const;

    private:

      void ParseStatus();

      void ParseHeader();

      void ParseContentLength();

      bool ParseBody();

    private:

      std::string mBytes;

      Status mStatus;

      std::vector<std::string> mHeader;

      size_t mContentLength;

      std::string mBody;

      std::mutex mMutex;
  };

  inline
  std::ostream& operator<< (std::ostream& Stream, const dl::http::Responce& Responce)
  {
    Stream << "HTTP/1.1 " << static_cast<int>(Responce.GetStatus()) << '\n';

    for (const auto& HeaderLine : Responce.GetHeader())
    {
      Stream << HeaderLine << '\n';
    }

    Stream << Responce.GetBody() << '\n';

    return Stream;
  }
}
