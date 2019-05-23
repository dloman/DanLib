//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP server, asynchronous
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace ip = boost::asio::ip; // from <boost/asio/ip/tcp.hpp>
namespace beast = boost::beast;
namespace http = beast::http;   // from <boost/beast/http.hpp>

// Return a reasonable mime type based on the extension of a file.
beast::string_view GetMimeType(beast::string_view Path)
{
  auto const ext = [&Path]
  {
    auto const Position = Path.rfind(".");
    if(Position == beast::string_view::npos)
    {
      return beast::string_view{};
    }
    return Path.substr(Position);
  }();

  using beast::iequals;

  if(iequals(ext, ".htm"))  return "text/html";
  if(iequals(ext, ".html")) return "text/html";
  if(iequals(ext, ".php"))  return "text/html";
  if(iequals(ext, ".css"))  return "text/css";
  if(iequals(ext, ".txt"))  return "text/plain";
  if(iequals(ext, ".js"))   return "application/javascript";
  if(iequals(ext, ".json")) return "application/json";
  if(iequals(ext, ".xml"))  return "application/xml";
  if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
  if(iequals(ext, ".flv"))  return "video/x-flv";
  if(iequals(ext, ".png"))  return "image/png";
  if(iequals(ext, ".jpe"))  return "image/jpeg";
  if(iequals(ext, ".jpeg")) return "image/jpeg";
  if(iequals(ext, ".jpg"))  return "image/jpeg";
  if(iequals(ext, ".gif"))  return "image/gif";
  if(iequals(ext, ".bmp"))  return "image/bmp";
  if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
  if(iequals(ext, ".tiff")) return "image/tiff";
  if(iequals(ext, ".tif"))  return "image/tiff";
  if(iequals(ext, ".svg"))  return "image/svg+xml";
  if(iequals(ext, ".svgz")) return "image/svg+xml";

  return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string ConcatenatePath(beast::string_view Base, beast::string_view Path)
{
  if(Base.empty())
  {
    return Path.to_string();
  }

  std::string Result = Base.to_string();
#if BOOST_MSVC
  char constexpr PathSeparator = '\\';

  if(Result.back() == PathSeparator)
  {
    Result.resize(Result.size() - 1);
  }
  Result.append(Path.data(), Path.size());

  for(auto& Character : Result)
  {
    if(Character == '/')
    {
      Character = PathSeparator;
    }
  }

#else
  char constexpr PathSeparator = '/';
  if(Result.back() == PathSeparator)
  {
    Result.resize(Result.size() - 1);
  }

  Result.append(Path.data(), Path.size());
#endif
  return Result;
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
template< class BodyType, class AllocatorType, class SendType>
void HandleRequest(
  beast::string_view DocumentRoot,
  http::request<BodyType, http::basic_fields<AllocatorType>>&& Request,
  SendType&& SendLambda)
{
  // Returns a bad request response
  auto const BadRequest =
    [&Request](beast::string_view Bad)
    {
      http::response<http::string_body> Responce{
        http::status::bad_request,
        Request.version()};

      Responce.set(http::field::server, BOOST_BEAST_VERSION_STRING);

      Responce.set(http::field::content_type, "text/html");

      Responce.keep_alive(Request.keep_alive());

      Responce.body() = Bad.to_string();

      Responce.prepare_payload();

      return Responce;
    };

  // Returns a not found response
  auto const not_found =
    [&Request](beast::string_view Target)
    {
      http::response<http::string_body> Responce{http::status::not_found, Request.version()};

      Responce.set(http::field::server, BOOST_BEAST_VERSION_STRING);

      Responce.set(http::field::content_type, "text/html");

      Responce.keep_alive(Request.keep_alive());

      Responce.body() = "The resource '" + Target.to_string() + "' was not found.";

      Responce.prepare_payload();

      return Responce;
    };

  // Returns a server error response
  auto const server_error =
    [&Request](beast::string_view What)
    {
      http::response<http::string_body> Responce{http::status::internal_server_error, Request.version()};

      Responce.set(http::field::server, BOOST_BEAST_VERSION_STRING);

      Responce.set(http::field::content_type, "text/html");

      Responce.keep_alive(Request.keep_alive());

      Responce.body() = "An error occurred: '" + What.to_string() + "'";

      Responce.prepare_payload();

      return Responce;
    };

  // Make sure we can handle the method
  if(
    Request.method() != http::verb::get &&
    Request.method() != http::verb::head)
  {
    return SendLambda(BadRequest("Unknown HTTP-method"));
  }

  // Request path must be absolute and not contain "..".
  if(
    Request.target().empty() ||
    Request.target()[0] != '/' ||
    Request.target().find("..") != beast::string_view::npos)
  {
    return SendLambda(BadRequest("Illegal request-target"));
  }

  // Build the path to the requested file
  std::string Path = ConcatenatePath(DocumentRoot, Request.target());

  if(Request.target().back() == '/')
  {
    Path.append("index.html");
  }

  // Attempt to open the file
  beast::error_code ErrorCode;

  http::file_body::value_type FileBody;

  FileBody.open(Path.c_str(), beast::file_mode::scan, ErrorCode);

  // Handle the case where the file doesn't exist
  if(ErrorCode == boost::system::errc::no_such_file_or_directory)
  {
    return SendLambda(not_found(Request.target()));
  }

  // Handle an unknown error
  if(ErrorCode)
  {
    return SendLambda(server_error(ErrorCode.message()));
  }

  // Cache the size since we need it after the move
  auto const Size = FileBody.size();

  // Respond to HEAD request
  if(Request.method() == http::verb::head)
  {
    http::response<http::empty_body> Responce{http::status::ok, Request.version()};

    Responce.set(http::field::server, BOOST_BEAST_VERSION_STRING);

    Responce.set(http::field::content_type, GetMimeType(Path));

    Responce.content_length(Size);

    Responce.keep_alive(Request.keep_alive());

    return SendLambda(std::move(Responce));
  }

  // Respond to GET request
  http::response<http::file_body> Responce{
    std::piecewise_construct,
      std::make_tuple(std::move(FileBody)),
      std::make_tuple(http::status::ok, Request.version())};

  Responce.set(http::field::server, BOOST_BEAST_VERSION_STRING);

  Responce.set(http::field::content_type, GetMimeType(Path));

  Responce.content_length(Size);

  Responce.keep_alive(Request.keep_alive());

  return SendLambda(std::move(Responce));
}

//------------------------------------------------------------------------------

// Report a failure
void Fail(boost::system::error_code ErrorCode, char const* What)
{
  std::cerr << What << ": " << ErrorCode.message() << "\n";
}

// Handles an HTTP server connection
class Session : public std::enable_shared_from_this<Session>
{
  // This is the C++11 equivalent of a generic lambda.
  // The function object is used to send an HTTP message.
  struct SendLambda
  {
    public:

      //------------------------------------------------------------------------
      //------------------------------------------------------------------------
      explicit SendLambda(Session& self)
      : mSelf(self)
      {
      }

      //------------------------------------------------------------------------
      //------------------------------------------------------------------------
      template<bool IsRequest, class Body, class Fields>
      void operator()(http::message<IsRequest, Body, Fields>&& Message) const
      {
        // The lifetime of the message has to extend
        // for the duration of the async operation so
        // we use a shared_ptr to manage it.
        auto pSharedMessage = std::make_shared<
          http::message<IsRequest, Body, Fields>>(std::move(Message));

        // Store a type-erased version of the shared
        // pointer in the class to keep it alive.
        mSelf.mResponce = pSharedMessage;

        // Write the response
        http::async_write(
          mSelf.mSocket,
          *pSharedMessage,
          boost::asio::bind_executor(
            mSelf.mStrand,
            std::bind(
              &Session::OnWrite,
              mSelf.shared_from_this(),
              std::placeholders::_1,
              std::placeholders::_2,
              pSharedMessage->need_eof())));
      }

      Session& mSelf;

  };

  public:
  // Take ownership of the socket
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  explicit Session(
    ip::tcp::socket socket,
    std::shared_ptr<std::string const> const& DocumentRoot)
  : mSocket(std::move(socket)),
    mStrand(mSocket.get_executor()),
    mDocumentRoot(DocumentRoot),
    mSendLambda(*this)
  {
  }

  // Start the asynchronous operation
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void Run()
  {
    DoRead();
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void DoRead()
  {
    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    mRequest = {};

    // Read a request
    http::async_read(mSocket, mBuffer, mRequest,
      boost::asio::bind_executor(
        mStrand,
        std::bind(
          &Session::OnRead,
          shared_from_this(),
          std::placeholders::_1,
          std::placeholders::_2)));
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void OnRead(boost::system::error_code ErrorCode, std::size_t BytesTransferred)
  {
    boost::ignore_unused(BytesTransferred);

    // This means they closed the connection
    if(ErrorCode == http::error::end_of_stream)
    {
      return DoClose();
    }

    if(ErrorCode)
    {
      return Fail(ErrorCode, "read");
    }

    // Send the response
    HandleRequest(*mDocumentRoot, std::move(mRequest), mSendLambda);
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void OnWrite(
    boost::system::error_code ErrorCode,
    std::size_t BytesTransferred,
    bool Close)
  {
    boost::ignore_unused(BytesTransferred);

    if(ErrorCode)
    {
      return Fail(ErrorCode, "write");
    }

    if(Close)
    {
      // This means we should Close the connection, usually because
      // the response indicated the "Connection: Close" semantic.
      return DoClose();
    }

    // We're done with the response so delete it
    mResponce = nullptr;

    // Read another request
    DoRead();
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void DoClose()
  {
    // Send a TCP shutdown
    boost::system::error_code ErrorCode;

    mSocket.shutdown(ip::tcp::socket::shutdown_send, ErrorCode);

    // At this point the connection is closed gracefully
  }

  private:

    ip::tcp::socket mSocket;

    boost::asio::strand<boost::asio::io_context::executor_type> mStrand;

    beast::flat_buffer mBuffer;

    std::shared_ptr<std::string const> mDocumentRoot;

    http::request<http::string_body> mRequest;

    std::shared_ptr<void> mResponce;

    SendLambda mSendLambda;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Accepts incoming connections and launches the sessions
class Listener : public std::enable_shared_from_this<Listener>
{
  public:

    Listener(
      boost::asio::io_context& IoContext,
      ip::tcp::endpoint Endpoint,
      std::shared_ptr<std::string const> const& DocumentRoot)
    : mAcceptor(IoContext),
      mSocket(IoContext),
      mDocumentRoot(DocumentRoot)
  {
    boost::system::error_code ErrorCode;

    // Open the acceptor
    mAcceptor.open(Endpoint.protocol(), ErrorCode);

    if(ErrorCode)
    {
      Fail(ErrorCode, "open");
      return;
    }

    // Allow address reuse
    mAcceptor.set_option(boost::asio::socket_base::reuse_address(true), ErrorCode);

    if(ErrorCode)
    {
      Fail(ErrorCode, "Set Option");
      return;
    }

    // Bind to the server address
    mAcceptor.bind(Endpoint, ErrorCode);
    if(ErrorCode)
    {
      Fail(ErrorCode, "bind");
      return;
    }

    // Start listening for connections
    mAcceptor.listen(
      boost::asio::socket_base::max_listen_connections, ErrorCode);

    if(ErrorCode)
    {
      Fail(ErrorCode, "listen");
      return;
    }
  }

  // Start accepting incoming connections
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void Run()
  {
    if(!mAcceptor.is_open())
    {
      return;
    }
    DoAccept();
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void DoAccept()
  {
    mAcceptor.async_accept(
      mSocket,
      std::bind(
        &Listener::OnAccept,
        shared_from_this(),
        std::placeholders::_1));
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void OnAccept(boost::system::error_code ErrorCode)
  {
    if(ErrorCode)
    {
      Fail(ErrorCode, "accept");
    }
    else
    {
      // Create the Session and run it
      std::make_shared<Session>(std::move(mSocket), mDocumentRoot)->Run();
    }

    // Accept another connection
    DoAccept();
  }

  private:

    ip::tcp::acceptor mAcceptor;

    ip::tcp::socket mSocket;

    std::shared_ptr<std::string const> mDocumentRoot;

};

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  // Check command line arguments.
  if (argc != 5)
  {
    std::cerr <<
      "Usage: http-server-async <address> <port> <DocumentRoot> <threads>\n" <<
      "Example:\n" <<
      "    http-server-async 0.0.0.0 8080 . 1\n";
    return EXIT_FAILURE;
  }
  auto const address = boost::asio::ip::make_address(argv[1]);
  auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
  auto const DocumentRoot = std::make_shared<std::string>(argv[3]);
  auto const threads = std::max<int>(1, std::atoi(argv[4]));

  // The io_context is required for all I/O
  boost::asio::io_context ioc{threads};

  // Create and launch a listening port
  std::make_shared<Listener>(
    ioc,
    ip::tcp::endpoint{address, port},
    DocumentRoot)->Run();

  // Run the I/O service on the requested number of threads
  std::vector<std::thread> v;
  v.reserve(threads - 1);
  for(auto i = threads - 1; i > 0; --i)
  {
    v.emplace_back(
      [&ioc]
        {
            ioc.run();
        });
  }
    ioc.run();

  return EXIT_SUCCESS;
}
