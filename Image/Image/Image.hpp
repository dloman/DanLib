#pragma once
#include <experimental/memory>
#include <memory>
#include <cstddef>
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
        std::unique_ptr<std::byte[]>&& Bytes,
        const size_t NumberOfChannels = 3);

      const std::experimental::observer_ptr<std::byte> GetData() const;

      std::experimental::observer_ptr<std::byte> GetData();

      size_t GetWidth() const;

      size_t GetHeight() const;

      size_t GetByteCount() const;

      size_t GetNumberOfChannels() const;

      void SetPixel(
        const size_t X,
        const size_t Y,
        const std::vector<std::byte>& Color);

      void DrawCircle(
        const int X,
        const int Y,
        const int Radius,
        const std::vector<std::byte>& Color);

      void DrawLine(
        int X1,
        int Y1,
        int X2,
        int Y2,
        const std::vector<std::byte>& Color);

      void DrawLine(
        int X1,
        int Y1,
        int X2,
        int Y2,
        const std::vector<std::byte>& Color,
        size_t Thickness);

    private:

      const size_t mWidth;

      const size_t mHeight;

      const size_t mNumberOfChannels;

      std::unique_ptr<std::byte[]> mpData;
  };
}
