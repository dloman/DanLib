#include "Image.hpp"
#include <cmath>

using dl::image::Image;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
double Distance(double X1, double Y1, double X2, double Y2)
{
  auto DistanceX = X1 - X2;
  auto DistanceY = Y1 - Y2;
  return std::sqrt((DistanceX * DistanceX) + (DistanceY * DistanceY));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Image::Image(const size_t Width, const size_t Height, const size_t NumberOfChannels)
  : mWidth(Width),
    mHeight(Height),
    mNumberOfChannels(NumberOfChannels),
    mpData(std::make_unique<std::uint8_t[]>(Width * Height * NumberOfChannels))
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Image::Image(
  const size_t Width,
  const size_t Height,
  std::unique_ptr<std::uint8_t[]>&& pData,
  const size_t NumberOfChannels)
  : mWidth(Width),
    mHeight(Height),
    mNumberOfChannels(NumberOfChannels),
    mpData(std::move(pData))
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const std::uint8_t* Image::GetData() const
{
  return mpData.get();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::uint8_t* Image::GetData()
{
  return mpData.get();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
size_t Image::GetWidth() const
{
  return mWidth;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
size_t Image::GetHeight() const
{
  return mHeight;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
size_t Image::GetByteCount() const
{
  return mNumberOfChannels * mWidth * mHeight;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
size_t Image::GetNumberOfChannels() const
{
  return mNumberOfChannels;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Image::SetPixel(size_t X, size_t Y, const std::vector<uint8_t>& Color)
{
  if (Color.size() != mNumberOfChannels)
  {
    throw std::logic_error("Invalid color in set pixel");
  }

  if (X < mWidth && Y < mHeight)
  {
    auto pData =
      mpData.get() + (mWidth * Y * mNumberOfChannels) + (X * mNumberOfChannels);

    *pData = Color[0];
    *(pData + 1) = Color[1];
    *(pData + 2) = Color[2];
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Image::DrawCircle(
  int X,
  int Y,
  int Radius,
  const std::vector<uint8_t>& Color)
{
  if (Color.size() != mNumberOfChannels)
  {
    throw std::logic_error("Invalid color in set pixel");
  }

  for (auto j = Y - Radius; j < Y + Radius; ++j)
  {
    for (auto i = X - Radius; i < X + Radius; ++i)
    {
      if (Distance(i, j, X, Y) <= Radius)
      {
        SetPixel(i, j, Color);
      }
    }
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Image::DrawLine(
  int X1,
  int Y1,
  int X2,
  int Y2,
  const std::vector<uint8_t>& Color)
{
  if (Color.size() != mNumberOfChannels)
  {
    throw std::logic_error("Invalid color in set pixel");
  }

  double LineDistance = Distance(X1, Y1, X2, Y2);

  double Run = (X2 - X1) / LineDistance;

  double Rise = (Y2 - Y1) / LineDistance;

  double X = X1, Y = Y1;

  for (int i = 0; i <= LineDistance; ++i)
  {
    SetPixel(X, Y, Color);

    X += Run;

    Y += Rise;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Image::DrawLine(
  int X1,
  int Y1,
  int X2,
  int Y2,
  const std::vector<uint8_t>& Color,
  size_t Thickness)
{
  double LineDistance = Distance(X1, Y1, X2, Y2);

  double Run = (X1 - X2) / LineDistance;

  double Rise = (Y1 - Y2) / LineDistance;

  if (Rise > Run)
  {
    Rise = 0;
  }
  else
  {
    Run = 0;
  }

  for (size_t j = 0; j < Thickness / 2; ++j)
  {
    DrawLine(X1 + (Rise * j), Y1 + (Run * j), X2 + (Rise * j), Y2 + (Run * j), Color);
    DrawLine(X1 - (Rise * j), Y1 - (Run * j), X2 - (Rise * j), Y2 - (Run * j), Color);
  }
}
