#include <concepts>
#include <type_traits>
namespace reaction {
  template <typename T, typename U>
  concept ConvertCC = std::is_convertible_v<std::decay_t<T>, std::decay_t<U>>;

  struct VarExpressionTag;
  template <typename T>
  concept VarExprCC = std::is_same_v<std::decay_t<T>, VarExpressionTag>;
}  // namespace reaction