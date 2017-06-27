#include <tuple>
#include <utility>
#include <type_traits>

namespace dl
{
  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename T, typename ... Args>
  constexpr bool ContainsType(std::tuple<Args...> t)
  {
    return std::disjunction_v<std::is_same<T, Args>...>;
  }
}
