/*
#include <Tcp/Client.hpp>
#include <iostream>
#include <array>

using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  cout << "Client connecting on localhost 8181" << endl;

  dl::tcp::Client<dl::tcp::Session> Client;

  Client.GetOnRxSignal().Connect([] (auto Bytes) { cout << Bytes << endl;});

  Client.GetOnDisconnectSignal().Connect(
    [] { cout << "server has disconnected" << endl;});

  while (true)
  {
    Client.Write(to_string(chrono::system_clock::now().time_since_epoch().count()));
    //std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  return 0;
}
*/
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

// Sends a WebSocket message and prints the response
int main(int argc, char** argv)
{
  try
  {
    // Check command line arguments.
    if(argc != 4)
    {
      std::cerr <<
        "Usage: websocket-client-sync <host> <port> <text>\n" <<
        "Example:\n" <<
        "    websocket-client-sync echo.websocket.org 80 \"Hello, world!\"\n";
      return EXIT_FAILURE;
    }
    auto const host = argv[1];
    auto const port = argv[2];
    auto const text = argv[3];

    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver{ioc};
    websocket::stream<tcp::socket> ws{ioc};

    // Look up the domain name
    auto const results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    boost::asio::connect(ws.next_layer(), results.begin(), results.end());

    // Perform the websocket handshake
    ws.handshake(host, "/");

    // Send the message
    ws.write(boost::asio::buffer(std::string(text)));

    // This buffer will hold the incoming message
    boost::beast::multi_buffer buffer;

    // Read a message into our buffer
    ws.read(buffer);
    std::cout << boost::beast::buffers(buffer.data()) << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));

    ws.read(buffer);
    std::cout << boost::beast::buffers(buffer.data()) << std::endl;

    // Close the WebSocket connection
    ws.close(websocket::close_code::normal);

    // If we get here then the connection is closed gracefully

    // The buffers() function helps print a ConstBufferSequence
    std::cout << boost::beast::buffers(buffer.data()) << std::endl;
  }
  catch(std::exception const& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

//]

