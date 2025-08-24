#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "resource.h"

namespace reaction {
  struct VarExpressionTag {};
  struct CalcExpressionTag {};

  template <typename Fun, typename... Args>
  class Expression : public Resource<ExpressionType<Fun, Args...>> {
   public:
    using ExprType = CalcExpressionTag;
    using ValueType = ExpressionType<Fun, Args...>;

    template <typename F, typename... A>
    Expression(F&& fun, A&&... args)
        : Resource<ExpressionType<Fun, Args...>>()
        , m_fun(std::forward<F>(fun))
        , m_args(std::forward<A>(args)...) {
      this->updateObservers(std::forward<A>(args)...);  // auto c = alc(functor, a, b); c订阅a,b
      evaluate();                                       // 计算当前值
    }

   private:
    void valueChanged() override {
      evaluate();      // 计算当前值
      this->notify();  // 通知观察者更新
    }
    // 实现观察者的更新策略
    void evaluate() {
      if constexpr (VoidType<ValueType>) {
        // action 没有返回值
        [&]<std::size_t... I>(std::index_sequence<I...>) {
          return std::invoke(m_fun, std::get<I>(m_args).get().get()...);
        }(std::make_index_sequence<std::tuple_size_v<decltype(m_args)>>{});
      } else {
        // lambda函数直接调用
        auto result =
            [&]<std::size_t... I>(std::index_sequence<I...>) {
              return std::invoke(m_fun, std::get<I>(m_args).get().get()...);  // get()*2???
            }(std::make_index_sequence<std::tuple_size_v<
                  decltype(m_args)>>{});  //{}构造一个临时对象,因为需要std::index_sequence类型
        this->updateValue(result);
      }
    }

   private:
    Fun m_fun;
    std::tuple<std::reference_wrapper<Args>...> m_args;
  };

  template <typename Type>
  class Expression<Type> : public Resource<Type> {
   public:
    using ExprType = VarExpressionTag;
    using ValueType = Type;
    using Resource<Type>::Resource;
  };
}  // namespace reaction