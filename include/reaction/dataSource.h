#include <type_traits>
#include <utility>

#include "expression.h"
#include "reaction/observerNode.h"

namespace reaction {
  template <typename Type, typename... Args>
  class DataSource : public Expression<Type, Args...> {
   public:
    using ExprType = typename Expression<Type, Args...>::ExprType;
    using ValueType = typename Expression<Type, Args...>::ValueType;
    using Expression<Type, Args...>::Expression;

    auto get() const { return this->getValue(); }

    template <typename T>
      requires ConvertCC<T, ValueType> && VarExprCC<VarExpressionTag>
    void value(T&& t) {
      this->updateValue(std::forward<T>(t));
      this->notify();
    }
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