#include <tuple>
#include <utility>
#include <type_traits>

namespace dl
{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename T, typename ... Args>
  constexpr bool ContainsType(std::tuple<Args...>)
  {
    return !std::is_same_v
    <
      std::integer_sequence<bool, false, std::is_same_v<T, Args>...>,
      std::integer_sequence<bool, std::is_same_v<T, Args>..., false>
    >;
  }
}
