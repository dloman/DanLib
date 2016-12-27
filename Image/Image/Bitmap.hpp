#pragma once

#include <memory>

namespace dl::image
{
  class Image;

  std::string CreateBitmap(const dl::image::Image& Image);

  void SaveBitmap(const dl::image::Image& Image, const std::string& Filename);
}

