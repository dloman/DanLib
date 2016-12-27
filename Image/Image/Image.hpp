#pragma once
#include <memory>
#include <stddef.h>
#include <vector>

namespace dl::image
{
  class Image
  {
    public:

      Image(const size_t Width, const size_t Height, const size_t NumberOfChannels = 3);

      Image(
        const size_t Width,
        const size_t Height,
        std::unique_ptr<std::uint8_t[]>&& Bytes,
        const size_t NumberOfChannels = 3);

      const std::uint8_t* GetData() const;

      std::uint8_t* GetData();

      size_t GetWidth() const;

      size_t GetHeight() const;

      size_t GetByteCount() const;

      size_t GetNumberOfChannels() const;

      void SetPixel(
        const size_t X,
        const size_t Y,
        const std::vector<uint8_t>& Color);

      void DrawCircle(
        const int X,
        const int Y,
        const int Radius,
        const std::vector<uint8_t>& Color);

      void DrawLine(
        int X1,
        int Y1,
        int X2,
        int Y2,
        const std::vector<uint8_t>& Color);

      void DrawLine(
        int X1,
        int Y1,
        int X2,
        int Y2,
        const std::vector<uint8_t>& Color,
        size_t Thickness);

    private:

      const size_t mWidth;

      const size_t mHeight;

      const size_t mNumberOfChannels;

      std::unique_ptr<std::uint8_t[]> mpData;
  };
}
