
#include <Requests/Requests.hpp>

#include <iostream>
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  try
  {
    //auto Request = dl::request::Get("www.images.halloweencostumes.com/products/4658/1-1/taco-dog-costume.jpg", 80);

    auto Request = dl::request::Get("http://www.theverge.com/google", 80);
    std::cout << Request.get() << std::endl;

    //Request = dl::request::Post("localhost", {{"taco", "burrito"}}, 8081);

    //std::cout << Request.get() << std::endl;
  }
  catch (const std::exception& Exception)
  {
    std::cerr << "ERROR: " << Exception.what() << std::endl;
  }

  return 0;
}
