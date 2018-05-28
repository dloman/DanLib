
#include <Websockets/Client.hpp>
#include <Websockets/Session.hpp>
#include <iostream>
#include <array>
#include <memory>
#include <thread>

using namespace std;

std::atomic<size_t> gDoneCount(0u);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Test()
{
  std::stringstream Stream("test");

  Stream << " " << std::this_thread::get_id();

  auto TestString = Stream.str();

  for (int j = 0; j < 100; ++j)
  {
    std::atomic<bool> Done(false);

    dl::ws::Client Client;

    Client.GetOnRxSignal().Connect
      ([] (const auto& Bytes) { cout << Bytes << endl;});

    Client.GetConnectionSignal().Connect(
      [&Client, &Done, TestString = std::move(TestString)]
      {
        for (int i = 0; i < 10; ++i)
        {
          Client.Write(TestString, dl::ws::DataType::eText);
        }

        Done = true;
      });

    Client.Connect();

    while(!Done)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
  }
  gDoneCount++;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  cout << "Clients connecting on localhost 8181" << endl;

  constexpr size_t NumberOfThreads = 100;

  std::array<std::unique_ptr<std::thread>, NumberOfThreads> Threads;

  for (size_t i = 0u; i < NumberOfThreads; ++i)
  {
    Threads[i] = std::make_unique<std::thread> (Test);
  }

  while(gDoneCount < NumberOfThreads)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  for (size_t i = 0u; i < NumberOfThreads; ++i)
  {
    Threads[i]->join();
  }

  return 0;
}
