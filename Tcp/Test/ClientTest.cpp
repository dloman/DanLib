
#include <Tcp/Client.hpp>
#include <iostream>

using namespace std;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  dl::tcp::Client TcpClient;

  cout << "Client connecting on localhost 8080" << endl;

  TcpClient.Connect();

  while (true)
  {
    this_thread::sleep_for(chrono::seconds(4));
    TcpClient.Write("foo");
    cout << "writing stuff" << endl;
  }
  return 0;
}


