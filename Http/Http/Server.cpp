#include "Server.hpp"
#include <Http/Request.hpp>
#include <String/String.hpp>

using dl::http::Server;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Server::Server(
  const unsigned Port,
  const unsigned NumberOfIoThreads,
  const unsigned NumberOfCallbackThreads)
: mServer(Port, 1, 1)
{
  mServer.GetNewSessionSignal().Connect(
    [this] (std::weak_ptr<dl::tcp::Session> pWeakSession)
    {
      auto pSession = pWeakSession.lock();

      if (pSession)
      {
        pSession->GetOnRxSignal().Connect(
          [this, pWeakSession, pRequest = std::make_shared<dl::http::Request>()]
          (const std::string& Bytes)
          {
            std::shared_ptr<dl::tcp::Session> pSession = pWeakSession.lock();

            if (pSession)
            {
              if (pRequest->AddBytes(Bytes))
              {
                pSession->Write(ParseRequest(*pRequest));
              }
            }
          });

        pSession->GetOnDisconnectSignal().Connect(
          [SessionId = pSession->GetSessionId()]
          {
            std::cout << "Session " << SessionId << " Disconnected" << std::endl;
          });
      }
    }
  );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string Server::ParseRequest(const dl::http::Request& Request)
{
  auto iCallback =
    mCallbacks.find({Request.GetRequestType(), Request.GetUrl()});

  if (iCallback != mCallbacks.end())
  {
    return iCallback->second(Request).ToBytes();
  }

  return dl::http::Responce::Error().ToBytes();
}
