#include <tuple>
#include <type_traits>

namespace dl
{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename T, typename Tuple>
  struct HasType;

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename T>
  struct HasType<T, std::tuple<>>
  : std::false_type
  {
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename T, typename U, typename... Args>
  struct HasType<T, std::tuple<U, Args...>>
  : HasType<T, std::tuple<Args...>>
  {
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename T, typename... Args>
  struct HasType<T, std::tuple<T, Args...>>
   : std::true_type
  {
  };

  template<typename T, typename Tuple>
  using HasType_t = typename HasType<T, Tuple>::type;
}
