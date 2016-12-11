#include <Http/Responce.hpp>
#include <future>
#include <unordered_map>

namespace dl::request
{
  std::future<dl::http::Responce> Get(
    const std::string& Url,
    const unsigned Port = 80);

  std::future<dl::http::Responce> Post(
    const std::string& Url,
    std::unordered_map<std::string, std::string> Data,
    const unsigned Port = 80);
}
