
#include <Requests/Requests.hpp>

using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  try
  {
    auto Request = dl::request::Get("example.com");

    std::cout << Request.get() << std::endl;
  }
  catch (const std::exception& Exception)
  {
    cerr << Exception.what() << std::endl;
  }

  return 0;
}
