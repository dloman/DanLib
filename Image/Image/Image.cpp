#include "Image.hpp"
#include <cmath>
#include <cstring>

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
Image::Image()
  : mWidth(0.0),
    mHeight(0.0),
    mColorSpace(ColorSpace::eRGB),
    mNumberOfChannels(0.0),
    mpOwnedData(nullptr),
    mpData(nullptr)
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Image::Image(
  size_t Width,
  size_t Height,
  ColorSpace colorSpace,
  size_t NumberOfChannels)
  : mWidth(Width),
    mHeight(Height),
    mColorSpace(ColorSpace::eRGB),
    mNumberOfChannels(NumberOfChannels),
    mpOwnedData(std::make_unique<std::byte[]>(Width * Height * NumberOfChannels)),
    mpData(std::experimental::make_observer(mpOwnedData.get()))
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Image::Image(
  size_t Width,
  size_t Height,
  std::unique_ptr<std::byte[]>&& pData,
  ColorSpace colorSpace,
  size_t NumberOfChannels)
  : mWidth(Width),
    mHeight(Height),
    mNumberOfChannels(NumberOfChannels),
    mpOwnedData(std::move(pData)),
    mpData(mpOwnedData.get())
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Image::Image(
  size_t Width,
  size_t Height,
  std::experimental::observer_ptr<std::byte> pBytes,
  ColorSpace colorSpace,
  size_t NumberOfChannels)
  : mWidth(Width),
    mHeight(Height),
    mColorSpace(colorSpace),
    mNumberOfChannels(NumberOfChannels),
    mpOwnedData(nullptr),
    mpData(pBytes)
{
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Image::~Image()
{
  volatile int a =6;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Image::Image(const Image& image)
  : mWidth(image.mWidth),
    mHeight(image.mHeight),
    mColorSpace(image.mColorSpace),
    mNumberOfChannels(image.mNumberOfChannels),
    mpOwnedData(std::make_unique<std::byte[]>(mWidth * mHeight * mNumberOfChannels)),
    mpData(mpOwnedData.get())
{
  std::memcpy(mpData.get(), image.mpData.get(), mWidth * mHeight * mNumberOfChannels);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Image& Image::operator = (const Image& rhs)
{

  mWidth = rhs.mWidth;

  mHeight = rhs.mHeight;

  mNumberOfChannels = rhs.mNumberOfChannels;

  mColorSpace = rhs.mColorSpace;

  mpOwnedData = std::make_unique<std::byte[]>(mWidth * mHeight * mNumberOfChannels);

  mpData = std::experimental::make_observer(mpOwnedData.get());

  memcpy(mpData.get(), rhs.mpData.get(), mWidth * mHeight * mNumberOfChannels);

  return *this;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Image::Image(Image&& image)
  : mWidth(image.mWidth),
    mHeight(image.mHeight),
    mColorSpace(image.mColorSpace),
    mNumberOfChannels(image.mNumberOfChannels),
    mpOwnedData(std::move(image.mpOwnedData)),
    mpData(std::move(image.mpData))
{
  image.mWidth = 0;
  image.mHeight = 0;
  image.mNumberOfChannels = 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Image& Image::operator = (Image&& rhs)
{

  mWidth = rhs.mWidth;

  mHeight = rhs.mHeight;

  mNumberOfChannels = rhs.mNumberOfChannels;

  mpOwnedData = std::move(rhs.mpOwnedData);

  mpData = std::move(rhs.mpData);

  rhs.mWidth = 0;
  rhs.mHeight = 0;
  rhs.mNumberOfChannels = 0;

  return *this;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Image::Set(
  const size_t width,
  const size_t height,
  std::unique_ptr<std::byte[]>&& pData,
  const size_t numberOfChannels)
{
  mWidth = width;

  mHeight = height;

  mpOwnedData = std::move(pData);

  mpData = std::experimental::make_observer(mpOwnedData.get());

  mNumberOfChannels = numberOfChannels;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::experimental::observer_ptr<const std::byte> Image::GetData() const
{
  return mpData;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::experimental::observer_ptr<std::byte> Image::GetMutableData()
{
  return mpData;
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
void Image::SetPixel(size_t X, size_t Y, const std::vector<std::uint8_t>& Color)
{
  if (Color.size() != mNumberOfChannels)
  {
    throw std::logic_error("Invalid color in set pixel");
  }

  if (X < mWidth && Y < mHeight)
  {
    auto pData =
      mpData.get() + (mWidth * Y * mNumberOfChannels) + (X * mNumberOfChannels);

    *pData = static_cast<std::byte>(Color[0]);
    *(pData + 1) = static_cast<std::byte>(Color[1]);
    *(pData + 2) = static_cast<std::byte>(Color[2]);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void Image::DrawCircle(
  int X,
  int Y,
  int Radius,
  const std::vector<std::uint8_t>& Color)
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
  const std::vector<std::uint8_t>& Color)
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
  const std::vector<std::uint8_t>& Color,
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
