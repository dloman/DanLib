
#include <Requests/Requests.hpp>

#include <iostream>
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  try
  {
    auto Request = dl::request::Get("www.example.com", 80);

    std::cout << Request.get() << std::endl;

    Request = dl::request::Post("localhost", {{"taco", "burrito"}}, 8081);

    std::cout << Request.get() << std::endl;
  }
  catch (const std::exception& Exception)
  {
    std::cerr << Exception.what() << std::endl;
  }

  return 0;
}
