#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "resource.h"

namespace reaction {
  template <typename T, typename... Args>
  class DataSource;

  template <typename T>
  struct ExpressionTraits {
    using Type = T;
  };

  template <typename T>
  struct ExpressionTraits<DataSource<T>> {
    using Type = T;
  };

  template <typename Fun, typename... Args>
  struct ExpressionTraits<DataSource<Fun, Args...>> {
    using Type = std::invoke_result_t<Fun, typename ExpressionTraits<Args>::Type...>;
  };

  template <typename Fun, typename... Args>
  using ExpressionType =
      typename ExpressionTraits<DataSource<Fun, Args...>>::Type;  // 避免typename来避免歧义

  template <typename Fun, typename... Args>
  class Expression : public Resource<ExpressionType<Fun, Args...>> {
   public:
    template <typename F, typename... A>
    Expression(F&& fun, A&&... args)
        : Resource<ExpressionType<Fun, Args...>>()
        , m_fun(std::forward<F>(fun))
        , m_args(std::forward<A>(args)...) {
      evaluate();
    }

   private:
    void evaluate() {
      auto result =
          [&]<std::size_t... I>(std::index_sequence<I...>) {
            return std::invoke(m_fun, std::get<I>(m_args).get().get()...);
          }(std::make_index_sequence<std::tuple_size_v<
                decltype(m_args)>>{});  //{}构造一个临时对象,因为需要std::index_sequence类型
      this->updateValue(result);
    }

   private:
    Fun m_fun;
    std::tuple<std::reference_wrapper<Args>...> m_args;
  };

  template <typename Type>
  class Expression<Type> : public Resource<Type> {
   public:
    using Resource<Type>::Resource;
  };
}  // namespace reaction