#include "Responce.hpp"
#include <String/String.hpp>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <numeric>

#include <iostream>
using dl::http::Responce;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
dl::http::Responce Responce::GenerateResponce(
  const Status status,
  const std::string& ContentType,
  const std::string& Body)
{
  dl::http::Responce Responce;

  std::time_t t = std::time(nullptr);
  std::stringstream Time;

  Time << "Date: " << std::put_time(std::gmtime(&t), "%c, %Z") << "\r\n";

  Responce.SetHeader({
    {"HTTP/1.1 " + std::to_string(static_cast<int> (status)) +" \r\n"},
    {Time.str()},
    {"Server: Apache/2.2.14 (Win32)\r\n"},
    {"Last-Modified: Sat, 20 Nov 2004 07:16:26 GMT\r\n"},
    {"ETag: \"10000000565a5-2c-3e94b66c2e680\"\r\n"},
    {"Accept-Ranges: bytes\r\n"},
    {"Content-Length: " + std::to_string(Body.size()) + "\r\n"},
    {"Connection: close\r\n"},
    {"Content-Type: " + ContentType + "\r\n"},
    {"X-Pad: avoid browser bug\r\n\r\n"}});

  Responce.SetBody(Body);

  return Responce;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
dl::http::Responce Responce::Error()
{
  return GenerateResponce(Status::eParsingError, "text/html", "error");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Responce::Responce()
  : mBytes(),
    mStatus(Status::eUninitialized),
    mHeader(),
    mContentLength(0),
    mBody(),
    mMutex()
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Responce::Responce(const Responce& Other)
  : mBytes(Other.mBytes),
    mStatus(Other.mStatus),
    mHeader(Other.mHeader),
    mBody(Other.mBody),
    mMutex()
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Responce::AddBytes(const std::string& Bytes)
{
  {
    std::lock_guard<std::mutex> Lock(mMutex);
    mBytes += Bytes;
  }

  if (mStatus == Status::eUninitialized)
  {
    ParseStatus();
  }

  if (mHeader.empty())
  {
    ParseHeader();
  }

  return ParseBody();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
dl::http::Responce::Status Responce::GetStatus() const
{
  return mStatus;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Responce::SetStatus(const dl::http::Responce::Status Status)
{
  mStatus = Status;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const std::vector<std::string>& Responce::GetHeader() const
{
  return mHeader;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const std::string& Responce::GetBody() const
{
  return mBody;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Responce::ParseStatus()
{
  std::lock_guard<std::mutex> Lock(mMutex);

  auto iLineBreak = mBytes.find("\r\n");

  if (iLineBreak != std::string::npos)
  {
    mStatus = static_cast<Status>(atoi(mBytes.substr(8, iLineBreak - 8).c_str()));

    mBytes = mBytes.substr(iLineBreak + 2);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Responce::ParseHeader()
{
  std::lock_guard<std::mutex> Lock(mMutex);
  auto iLineBreak = mBytes.find("\r\n\r\n");

  if (iLineBreak != std::string::npos)
  {
    mHeader = dl::Split(mBytes.substr(0, iLineBreak));

    ParseContentLength();

    mBytes = mBytes.substr(iLineBreak + 4);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Responce::ParseContentLength()
{
  auto iLine = std::find_if(
    mHeader.begin(),
    mHeader.end(),
    [] (const std::string& Line)
    {
      return Line.substr(0, 16) == "Content-Length: ";
    });;

  if (iLine == mHeader.end())
  {
    mContentLength = 1;
    mBytes += ' ';
  }
  else
  {
    mContentLength = std::atoi(iLine->substr(16).c_str());
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Responce::ParseBody()
{
  std::lock_guard<std::mutex> Lock(mMutex);
  if (mBytes.size() >= mContentLength)
  {
    mBody = std::move(mBytes);

    return true;
  }
  return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Responce::SetBody(const std::string& Body)
{
  mBody = Body;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Responce::SetHeader(const std::vector<std::string>&& Header)
{
  mHeader = std::move(Header);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string Responce::ToBytes() const
{
  std::string Responce;

  Responce = std::accumulate(mHeader.begin(), mHeader.end(), Responce);

  return Responce + mBody;
}
