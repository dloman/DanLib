
#include <Tcp/Client.hpp>
#include <iostream>

using namespace std;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  cout << "Client connecting on localhost 8080" << endl;

  dl::tcp::Client TcpClient;

  while (true)
  {
    TcpClient.Write("foo");
    //cout << "writing stuff" << endl;
    this_thread::sleep_for(chrono::milliseconds(1200));
  }
  return 0;
}
