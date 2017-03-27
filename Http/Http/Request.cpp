#include "Request.hpp"
#include <String/String.hpp>
#include <algorithm>
#include <experimental/string_view>

#include <iostream>
using dl::http::Request;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Request::Request()
  : mBytes(),
    mRequestType(dl::http::RequestType::eUnknown),
    mUrl("/"),
    mHeader(),
    mContentLength(0),
    mBody(),
    mMutex()
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Request::Request(const Request& Other)
  : mBytes(Other.mBytes),
    mRequestType(Other.mRequestType),
    mUrl(Other.mUrl),
    mHeader(Other.mHeader),
    mBody(Other.mBody),
    mMutex()
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Request::AddBytes(const std::string& Bytes)
{
  {
    std::lock_guard<std::mutex> Lock(mMutex);
    mBytes += Bytes;
  }

  if (mHeader.empty())
  {
    ParseHeader();
  }

  if (mRequestType == dl::http::RequestType::eUnknown)
  {
    ParseRequest();
  }

  return ParseBody();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
dl::http::RequestType Request::GetRequestType() const
{
  return mRequestType;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Request::SetRequestType(const dl::http::RequestType RequestType)
{
  mRequestType = RequestType;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const std::vector<std::string>& Request::GetHeader() const
{
  return mHeader;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const std::string& Request::GetBody() const
{
  return mBody;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const std::string& Request::GetUrl() const
{
  return mUrl;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Request::ParseHeader()
{
  std::lock_guard<std::mutex> Lock(mMutex);
  auto iLineBreak = mBytes.find("\r\n\r\n");

  if (iLineBreak != std::string::npos)
  {
    mHeader = dl::Split(mBytes.substr(0, iLineBreak));

    ParseRequest();

    mBytes = mBytes.substr(iLineBreak + 4);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static dl::http::RequestType GenerateRequestType(
  std::experimental::string_view requestType)
{
  if (!requestType.compare("GET"))
  {
    return dl::http::RequestType::eGet;
  }
  else if (!requestType.compare("POST"))
  {
    return dl::http::RequestType::ePost;
  }

  return dl::http::RequestType::eUnknown;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Request::ParseRequest()
{
  auto FirstLine = dl::Split(mHeader[0], ' ');

  if (FirstLine.size() < 2)
  {
    mRequestType = dl::http::RequestType::eFail;

    return;
  }

  mRequestType = GenerateRequestType(FirstLine[0]);

  mUrl = FirstLine[1];
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Request::ParseBody()
{
  std::lock_guard<std::mutex> Lock(mMutex);
  if (mBytes.size() >= mContentLength)
  {
    mBody = std::move(mBytes);

    return true;
  }
  return false;
}

