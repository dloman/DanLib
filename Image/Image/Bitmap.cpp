#include "Bitmap.hpp"
#include <Image/Image.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>

namespace dl::image
{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  static std::vector<std::byte> GeneratePixelData(
    const dl::image::Image& Image)
  {
    auto GetPixel =
      [pData = Image.GetData(), Width = Image.GetWidth(), Bitdepth = Image.GetNumberOfChannels()]
      (const size_t x, const size_t y)
      {
        return pData.get() + (y * Width * Bitdepth) + (x * Bitdepth);
      };

    std::vector<std::byte> Data;

    for (size_t y = Image.GetHeight(); y > 0; --y)
    {
      for (size_t x = 0; x < Image.GetWidth(); ++x)
      {
        auto pData = GetPixel(x, y);

        Data.push_back(*(pData + 2));

        Data.push_back(*(pData + 1));

        Data.push_back(*pData);
      }
      //Pad to each row to be multiple of 4
      while (Data.size() % 4)
      {
        Data.emplace_back(static_cast<std::byte>(0));
      }
    }

    return Data;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename Type>
  static void Append(Type Value, std::vector<uint8_t>& Data)
  {
    std::array<uint8_t, sizeof(Type)> Bytes;

    memcpy(Bytes.data(), &Value, sizeof(Type));

    for (size_t i = 0; i < sizeof(Type); ++i)
    {
      Data.push_back(Bytes[i]);
    }
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void SaveBitmap(const dl::image::Image& Image, const std::string& Filename)
  {
    auto Bitmap = CreateBitmap(Image);

    std::ofstream File(Filename);

    File << Bitmap;
  }


  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  std::string CreateBitmap(const dl::image::Image& Image)
  {
    auto PixelData = GeneratePixelData(Image);

    std::vector<uint8_t> Header;

    Header.emplace_back(66); //ASCII B

    Header.emplace_back(77); //ASCII M

    Append<uint32_t>(54 + PixelData.size(), Header); // Total size

    Append<uint32_t>(0, Header); //

    Append<uint32_t>(0, Header);

    Append<uint32_t>(40, Header); //BITMAPINFOHEADER size

    Append<uint32_t>(Image.GetWidth(), Header);

    Append<uint32_t>(Image.GetHeight(), Header);

    Append<uint16_t>(1, Header); //Color

    Append<uint16_t>(8 * Image.GetNumberOfChannels(), Header);

    Append<uint32_t>(0, Header); //Disable Compression

    Append<uint32_t>(PixelData.size(), Header);

    Append<uint32_t>(2835, Header); //Horizontal resolition

    Append<uint32_t>(2835, Header); //Vertical resolution

    Append<uint32_t>(0, Header); //number of colors

    Append<uint32_t>(0, Header); //number of colors

    std::stringstream Output;

    Output.write(reinterpret_cast<const char*> (Header.data()), Header.size());

    Output.write(reinterpret_cast<const char*> (PixelData.data()), PixelData.size());

    return Output.str();
  }
}
