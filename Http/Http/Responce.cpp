#include "Responce.hpp"

using dl::http::Responce;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Responce::Responce()
  : mBytes(),
    mStatus(Status::eUninitialized),
    mHeader(),
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

  if (mBody.empty())
  {
    return ParseBody();
  }

  return false;
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
const std::string& Responce::GetHeader() const
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
    mHeader = mBytes.substr(0, iLineBreak);

    mBytes = mBytes.substr(iLineBreak + 4);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool Responce::ParseBody()
{
  std::lock_guard<std::mutex> Lock(mMutex);
  auto iLineBreak = mBytes.find("\r\n\r\n");

  if (iLineBreak != std::string::npos)
  {
    mBody = std::move(mBytes);

    return true;
  }
  return false;
}
