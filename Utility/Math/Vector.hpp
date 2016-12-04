#pragma once

#include <array>
#include <iostream>
#include <memory>
#include <stddef.h>
#include <type_traits>

namespace dl::math
{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename ExpressionType, size_t ExpressionSize>
  class Expression
  {
    public:

      auto operator[](size_t i) const
      {
        return static_cast<ExpressionType&>(*this)[i];
      }

      const ExpressionType& operator()() const
      {
        return static_cast<const ExpressionType&>(*this);
      }

      static constexpr size_t Size = ExpressionSize;
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename LeftType, typename RightType>
  class Sum : public Expression<Sum<LeftType, RightType>, LeftType::Size>
  {
    public:

      Sum(const LeftType& Lhs, const RightType& Rhs)
        : mLhs(Lhs),
          mRhs(Rhs)
      {
        static_assert(
          LeftType::Size == RightType::Size,
          "error Size mismatch");
      }

      typename LeftType::value_type operator[](size_t i) const
      {
        return mLhs[i] + mRhs[i];
      }

      static_assert(
        std::is_convertible<typename LeftType::value_type, typename RightType::value_type>::value,
        "error must use similar values in sum");

      using value_type = typename LeftType::value_type;

    private:

      const LeftType& mLhs;

      const RightType& mRhs;
  };


  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename LeftType, typename RightType>
  const Sum<LeftType, RightType> operator + (
    const LeftType& Lhs,
    const RightType& Rhs)
  {
    return Sum<LeftType, RightType>(Lhs, Rhs);
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename ValueType, size_t Size>
  class Vector : public Expression<Vector<ValueType, Size>, Size>
  {
    public:

      using value_type = ValueType;

      Vector()
        : mpData(std::make_unique<std::array<ValueType, Size>>())
      {
      }

      Vector(const std::array<ValueType, Size>& Data)
        : mpData(std::make_unique<std::array<ValueType, Size>>(Data))
      {
      }

      template <typename ExpressionType>
      Vector(const ExpressionType& ExpressionArg)
        : mpData(std::make_unique<std::array<ValueType, Size>> ())
      {
        for (auto i = 0u; i < Size; ++i)
        {
          (*mpData)[i] = ExpressionArg[i];
        }
      }

      ValueType& operator[] (size_t Index)
      {
        return (*mpData)[Index];
      }

      const ValueType& operator[] (size_t Index) const
      {
        return (*mpData)[Index];
      }

      ValueType* begin()
      {
        return mpData.get()->data();
      }

      ValueType* end()
      {
        return mpData.get()->data() + Size;
      }

    private:

      std::unique_ptr<std::array<ValueType, Size>> mpData;
  };
}
