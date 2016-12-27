
#include <Requests/Requests.hpp>

#include <iostream>
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  try
  {
    //auto Request = dl::request::Get("http://www.theverge.com/google", 80);

    //std::cout << Request.get() << std::endl;

    //Request = dl::request::Post("localhost", {{"taco", "burrito"}}, 80);
    auto Request = dl::request::Get("localhost", 80);

    std::cout << Request.get() << std::endl;
  }
  catch (const std::exception& Exception)
  {
    std::cerr << "ERROR: " << Exception.what() << std::endl;
  }

  return 0;
}
