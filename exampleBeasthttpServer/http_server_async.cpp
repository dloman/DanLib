//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
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
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <boost/filesystem/path.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace asio = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// Return a reasonable mime type based on the extension of a file.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
beast::string_view GetMimeType(const boost::filesystem::path& Path)
{
  using beast::iequals;

  auto const Extension = Path.extension().string();

  if(iequals(Extension, ".htm"))  return "text/html";
  if(iequals(Extension, ".html")) return "text/html";
  if(iequals(Extension, ".php"))  return "text/html";
  if(iequals(Extension, ".css"))  return "text/css";
  if(iequals(Extension, ".txt"))  return "text/plain";
  if(iequals(Extension, ".js"))   return "application/javascript";
  if(iequals(Extension, ".json")) return "application/json";
  if(iequals(Extension, ".xml"))  return "application/xml";
  if(iequals(Extension, ".swf"))  return "application/x-shockwave-flash";
  if(iequals(Extension, ".flv"))  return "video/x-flv";
  if(iequals(Extension, ".png"))  return "image/png";
  if(iequals(Extension, ".jpe"))  return "image/jpeg";
  if(iequals(Extension, ".jpeg")) return "image/jpeg";
  if(iequals(Extension, ".jpg"))  return "image/jpeg";
  if(iequals(Extension, ".gif"))  return "image/gif";
  if(iequals(Extension, ".bmp"))  return "image/bmp";
  if(iequals(Extension, ".ico"))  return "image/vnd.microsoft.icon";
  if(iequals(Extension, ".tiff")) return "image/tiff";
  if(iequals(Extension, ".tif"))  return "image/tiff";
  if(iequals(Extension, ".svg"))  return "image/svg+xml";
  if(iequals(Extension, ".svgz")) return "image/svg+xml";
  return "application/text";
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
template<class BodyType, class AllocatorType, class SendType>
void HandleRequest(
  const boost::filesystem::path& DocumentRoot,
  http::request<BodyType, http::basic_fields<AllocatorType>>&& Request,
  SendType&& SendResponce)
{
  auto const GenerateResponce =
    [&Request](http::status Status, beast::string_view Message)
    {
      http::response<http::string_body> Responce{Status, Request.version()};

      Responce.set(http::field::server, BOOST_BEAST_VERSION_STRING);

      Responce.set(http::field::content_type, "text/html");

      Responce.keep_alive(Request.keep_alive());

      Responce.body() = Message.to_string();

      Responce.prepare_payload();

      return Responce;
    };

  // Returns a bad request response
  auto const BadRequest =
    [&Request, &GenerateResponce](beast::string_view Why)
    {
      return GenerateResponce(http::status::bad_request, Why);
    };

  // Returns a not found response
  auto const NotFound = [&Request, &GenerateResponce](beast::string_view Target)
    {
      return GenerateResponce(
        http::status::not_found,
        "The resource '" + std::string(Target) + "' was not found.");
    };

  // Returns a server error response
  auto const ServerError = [&Request, &GenerateResponce](beast::string_view What)
    {
      return GenerateResponce(
        http::status::internal_server_error,
        "An error occurred: '" + std::string(What) + "'");
    };

  // Make sure we can handle the method
  if(
    Request.method() != http::verb::get &&
    Request.method() != http::verb::head)
  {
    return SendResponce(BadRequest("Unknown HTTP-method"));
  }

  // Request Path must be absolute and not contain "..".
  if(
    Request.target().empty() ||
    Request.target()[0] != '/' ||
    Request.target().find("..") != beast::string_view::npos)
  {
    return SendResponce(BadRequest("Illegal request-target"));
  }

  // Build the Path to the requested file
  auto Path = DocumentRoot / boost::filesystem::path(Request.target().to_string());
  if(Request.target().back() == '/')
  {
    Path /= "index.html";
  }

  // Attempt to open the file
  beast::error_code ErrorCode;
  http::file_body::value_type Body;
  Body.open(Path.c_str(), beast::file_mode::scan, ErrorCode);

  // Handle the case where the file doesn't exist
  if(ErrorCode == beast::errc::no_such_file_or_directory)
  {
    return SendResponce(NotFound(Request.target()));
  }

  // Handle an unknown error
  if(ErrorCode)
  {
    return SendResponce(ServerError(ErrorCode.message()));
  }

  // Cache the size since we need it after the move
  auto const Size = Body.size();

  // Respond to HEAD request
  if(Request.method() == http::verb::head)
  {
    http::response<http::empty_body> Responce{http::status::ok, Request.version()};

    Responce.set(http::field::server, BOOST_BEAST_VERSION_STRING);

    Responce.set(http::field::content_type, GetMimeType(Path));

    Responce.content_length(Size);

    Responce.keep_alive(Request.keep_alive());

    return SendResponce(std::move(Responce));
  }

  // Respond to GET request
  http::response<http::file_body> Responce{
    std::piecewise_construct,
      std::make_tuple(std::move(Body)),
      std::make_tuple(http::status::ok, Request.version())};

  Responce.set(http::field::server, BOOST_BEAST_VERSION_STRING);

  Responce.set(http::field::content_type, GetMimeType(Path));

  Responce.content_length(Size);

  Responce.keep_alive(Request.keep_alive());

  return SendResponce(std::move(Responce));
}

// Report a failure
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void ReportFailure(beast::error_code ErrorCode, char const* What)
{
  std::cerr << What << ": " << ErrorCode.message() << "\n";
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Handles an HTTP server connection
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class Session : public std::enable_shared_from_this<Session>
{
  // This is the C++11 equivalent of a generic lambda.
  // The function object is used to send an HTTP message.
  struct GenericSender
  {
    Session& mSelf;

    explicit GenericSender(Session& Self)
    : mSelf(Self)
    {
    }

    template<bool IsRequestType, class BodyType, class FieldsType>
    void operator()(http::message<IsRequestType, BodyType, FieldsType>&& Message) const
    {
      // The lifetime of the message has to extend
      // for the duration of the async operation so
      // we use a shared_ptr to manage it.
      auto pResponce = std::make_shared<http::message<IsRequestType, BodyType, FieldsType>>(
        std::move(Message));

      // Store a type-erased version of the shared
      // pointer in the class to keep it alive.
      mSelf.mpResponce = pResponce;

      // Write the response
      http::async_write(
        mSelf.mStream,
        *pResponce,
        beast::bind_front_handler(
          &Session::OnWrite,
          mSelf.shared_from_this(),
          pResponce->need_eof()));
    }
  };

  beast::tcp_stream mStream;

  beast::flat_buffer mBuffer;

  const boost::filesystem::path mDocumentRoot;

  http::request<http::string_body> mRequest;

  std::shared_ptr<void> mpResponce;

  GenericSender mLambda;

  public:
  // Take ownership of the stream
  Session(
    tcp::socket&& Socket,
    const boost::filesystem::path& DocumentRoot)
    : mStream(std::move(Socket)),
      mDocumentRoot(DocumentRoot),
      mLambda(*this)
  {
  }

  // Start the asynchronous operation
  void Run()
  {
    DoRead();
  }

  void DoRead()
  {
    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    mRequest = {};

    // Set the timeout.
    mStream.expires_after(std::chrono::seconds(30));

    // Read a request
    http::async_read(
      mStream,
      mBuffer,
      mRequest,
      beast::bind_front_handler(
        &Session::OnRead,
        shared_from_this()));
  }

  void OnRead(beast::error_code ErrorCode, std::size_t BytesTransferred)
  {
    boost::ignore_unused(BytesTransferred);

    // This means they closed the connection
    if(ErrorCode == http::error::end_of_stream)
    {
      return DoClose();
    }

    if(ErrorCode)
    {
      return ReportFailure(ErrorCode, "read");
    }

    // Send the response
    HandleRequest(mDocumentRoot, std::move(mRequest), mLambda);
  }

  void OnWrite(
    bool Close,
    beast::error_code ErrorCode,
    std::size_t BytesTransferred)
  {
    boost::ignore_unused(BytesTransferred);

    if(ErrorCode)
    {
      return ReportFailure(ErrorCode, "write");
    }

    if(Close)
    {
      // This means we should close the connection, usually because
      // the response indicated the "Connection: close" semantic.
      return DoClose();
    }

    // We're done with the response so delete it
    mpResponce = nullptr;

    // Read another request
    DoRead();
  }

  void DoClose()
  {
    // Send a TCP shutdown
    beast::error_code ErrorCode;
    mStream.socket().shutdown(tcp::socket::shutdown_send, ErrorCode);

    // At this point the connection is closed gracefully
  }
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Accepts incoming connections and launches the sessions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class Server : public std::enable_shared_from_this<Server>
{
  asio::io_context mIoContext;
  tcp::acceptor mAcceptor;
  const boost::filesystem::path mDocumentRoot;
  std::vector<std::thread> mThreads;

  public:
  Server(
    size_t NumberOfThreads,
    tcp::endpoint Endpoint,
    const boost::filesystem::path& DocumentRoot)
  : mIoContext(),
    mAcceptor(asio::make_strand(mIoContext)),
    mDocumentRoot(DocumentRoot),
    mThreads()
  {
    // Run the I/O service on the requested number of Threads
    mThreads.reserve(NumberOfThreads);

    beast::error_code ErrorCode;

    // Open the acceptor
    mAcceptor.open(Endpoint.protocol(), ErrorCode);

    if(ErrorCode)
    {
      ReportFailure(ErrorCode, "open");
      return;
    }

    // Allow Address reuse
    mAcceptor.set_option(asio::socket_base::reuse_address(true), ErrorCode);

    if(ErrorCode)
    {
      ReportFailure(ErrorCode, "set_option");
      return;
    }

    // Bind to the server Address
    mAcceptor.bind(Endpoint, ErrorCode);

    if(ErrorCode)
    {
      ReportFailure(ErrorCode, "bind");
      return;
    }

    // Start listening for connections
    mAcceptor.listen(
      asio::socket_base::max_listen_connections, ErrorCode);

    if(ErrorCode)
    {
      ReportFailure(ErrorCode, "listen");
      return;
    }

    for(auto i = NumberOfThreads; i > 0; --i)
    {
      mThreads.emplace_back(
        [this]
        {
          mIoContext.run();
        });
    }

  }

  ~Server()
  {
    for (auto& Thread : mThreads)
    {
      Thread.join();
    }
  }
  // Start accepting incoming connections
  void Run()
    {
      DoAccept();
    }

  private:
  void
    DoAccept()
    {
      // The new connection gets its own strand
      mAcceptor.async_accept(
        asio::make_strand(mIoContext),
        beast::bind_front_handler(
          &Server::OnAccept,
          shared_from_this()));
    }

  void OnAccept(beast::error_code ErrorCode, tcp::socket Socket)
  {
    if(ErrorCode)
    {
      ReportFailure(ErrorCode, "accept");
    }
    else
    {
      // Create the Session and Run it
      std::make_shared<Session>(std::move(Socket), mDocumentRoot)->Run();
    }

    // Accept another connection
    DoAccept();
  }
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  // Check command line arguments.
  if (argc != 4)
  {
    std::cerr <<
      "Usage: http-server-async <Address> <Port> <DocumentRoot>\n" <<
      "Example:\n" <<
      "    http-server-async 0.0.0.0 8080 . 1\n";
    return EXIT_FAILURE;
  }

  auto const Address = asio::ip::make_address(argv[1]);
  auto const Port = static_cast<unsigned short>(std::atoi(argv[2]));
  auto const DocumentRoot = boost::filesystem::path(argv[3]);

  // Create and launch a listening Port
  std::make_shared<Server>(
    2u,
    tcp::endpoint{Address, Port},
    DocumentRoot)->Run();

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }

  return EXIT_SUCCESS;
}
