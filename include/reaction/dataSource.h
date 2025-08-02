#include <type_traits>
#include <utility>

#include "expression.h"

namespace reaction {
  template <typename T, typename... Args>
  class DataSource : public Expression<T, Args...> {
   public:
    using Expression<T, Args...>::Expression;
    auto get() const { return this->getValue(); }
  };

  template <typename T>
  auto var(T&& t) {
    return DataSource<T>(std::forward<T>(t));
  }

  template <typename Func, typename... Args>
  auto calc(Func&& fun, Args&&... args) {
    return DataSource<std::decay_t<Func>, std::decay_t<Args>...>(std::forward<Func>(fun),
                                                                 std::forward<Args>(args)...);
  }
}  // namespace reaction