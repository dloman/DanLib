#include <tuple>
#include <utility>
#include <type_traits>

namespace dl
{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename T, typename Tuple>
  struct ContainsType;

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename T>
  struct ContainsType<T, std::tuple<>>
  : std::false_type
  {
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename T, typename U, typename... Args>
  struct ContainsType<T, std::tuple<U, Args...>>
  : ContainsType<T, std::tuple<Args...>>
  {
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename T, template<typename> class U, typename ... Ts, typename... Args>
  struct ContainsType<T, std::tuple<U<Ts...>, Args...>>
  : ContainsType<T, std::tuple<Ts..., Args...>>
  {
  };

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename T, typename... Args>
  struct ContainsType<T, std::tuple<T, Args...>>
  : std::true_type
  {
  };

  template<typename T, typename Tuple>
  using contains_type_t = typename ContainsType<T, Tuple>::type;

  template<typename T, typename Tuple>
  using contains_type_v = typename ContainsType<T, Tuple>::value;
}
