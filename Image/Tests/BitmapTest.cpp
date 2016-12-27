#include <Image/Bitmap.hpp>
#include <Image/Image.hpp>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main()
{
  dl::image::Image Image(400, 400);

  for (size_t i = 0; i < 400; ++i)
  {
    if (i < 200)
    {
      Image.SetPixel(i, i, {0, 0, 0});
    }
    else
    {
      Image.SetPixel(i, i, {255, 255, 255});
    }
  }

  dl::image::SaveBitmap(Image, "output.bmp");

  return 0;
}
