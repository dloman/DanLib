#include <Udp/Server.hpp>
#include <condition_variable>
#include <csignal>
#include <iostream>

std::condition_variable gConditionVariable;
std::mutex gMutex;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Interrupt(int)
{
  gConditionVariable.notify_all();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  std::signal(SIGINT, Interrupt);

  std::cout << "Listening on port 1337" << std::endl;

  dl::udp::Server Server(1337);

  Server.GetOnRxSignal().Connect(
    [&Server] (const auto& Bytes, const auto& IpAddress)
    {
      std::cout << "rx = " << Bytes << " from " << IpAddress << std::endl;

      Server.Write(Bytes, IpAddress);
    });

  std::unique_lock<std::mutex> Lock(gMutex);
  gConditionVariable.wait(Lock);

  std::cout << "goodbye" << std::endl;

  return 0;
}

