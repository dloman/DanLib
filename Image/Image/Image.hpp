#pragma once
#include <experimental/memory>
#include <memory>
#include <cstddef>
#include <vector>

namespace dl::image
{
  enum class ColorSpace
  {
    eRGB,
    eYUV
  };

  class Image
  {
    public:

      Image();

      Image(
        size_t Width,
        size_t Height,
        ColorSpace colorSpace = ColorSpace::eRGB,
        size_t NumberOfChannels = 3);

      Image(
        size_t Width,
        size_t Height,
        std::unique_ptr<std::byte[]>&& Bytes,
        ColorSpace colorSpace = ColorSpace::eRGB,
        size_t NumberOfChannels = 3);

      Image(
        size_t Width,
        size_t Height,
        std::experimental::observer_ptr<std::byte> Bytes,
        ColorSpace colorSpace = ColorSpace::eRGB,
        size_t NumberOfChannels = 3);

      Image(const Image& image);

      Image& operator = (const Image& Rhs);

      Image(Image&& image);

      Image& operator = (Image&& Rhs);

      std::experimental::observer_ptr<const std::byte> GetData() const;

      std::experimental::observer_ptr<std::byte> GetMutableData();

      void Set(
        const size_t width,
        const size_t height,
        std::unique_ptr<std::byte[]>&& pData,
        const size_t numberOfChannels);

      size_t GetWidth() const;

      size_t GetHeight() const;

      size_t GetByteCount() const;

      size_t GetNumberOfChannels() const;

      void SetPixel(
        const size_t X,
        const size_t Y,
        const std::vector<std::uint8_t>& Color);

      void DrawCircle(
        const int X,
        const int Y,
        const int Radius,
        const std::vector<std::uint8_t>& Color);

      void DrawLine(
        int X1,
        int Y1,
        int X2,
        int Y2,
        const std::vector<std::uint8_t>& Color);

      void DrawLine(
        int X1,
        int Y1,
        int X2,
        int Y2,
        const std::vector<std::uint8_t>& Color,
        size_t Thickness);

      ColorSpace GetColorSpace() const;

      void SetColorSpace(ColorSpace colorspace);

    private:

      size_t mWidth;

      size_t mHeight;

      ColorSpace mColorSpace;

      size_t mNumberOfChannels;

      std::unique_ptr<std::byte[]> mpOwnedData;

      std::experimental::observer_ptr<std::byte> mpData;
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  inline
  ColorSpace Image::GetColorSpace() const
  {
    return mColorSpace;
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  inline
  void Image::SetColorSpace(ColorSpace colorSpace)
  {
    mColorSpace = colorSpace;
  }
}
