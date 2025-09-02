#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "reaction/observerNode.h"
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
    void setSource(F&& fun, A&&... args) {
      if constexpr (std::convertible_to<ExpressionType<std::decay_t<F>, std::decay_t<A>...>,
                                        ValueType>) {
        this->updateObservers(args.getSharedPtr()...);  // 订阅args的变化
        setFunctor(createFun(std::forward<F>(fun), std::forward<A>(args)...));
        evaluate();
      }
    }
    // 给()使用
    void addObCb(NodePtr node) { this->updateObservers(node); }

   private:
    template <typename F, typename... A>
    auto createFun(F&& fun, A&&... args) {
      return [fun = std::forward<F>(fun), ... args = args.getSharedPtr()]() {
        if constexpr (VoidType<ValueType>) {
          std::invoke(fun, args->get()...);
          return VoidWrapper{};
        } else {
          return std::invoke(fun, args->get()...);
        }
      };
    }

    void valueChanged() override {
      evaluate();
      this->notify();  // 通知观察者更新
    }
    // 实现观察者的更新策略
    void evaluate() {
      if constexpr (VoidType<ValueType>) {
        std::invoke(m_fun);  // 参数均在捕获列表里
      } else {
        this->updateValue(std::invoke(m_fun));
      }
    }

    void setFunctor(const std::function<ValueType()>& fun) { m_fun = fun; }

   private:
    std::function<ValueType()> m_fun;  // 用于reset
  };

  template <NonInvocableType Type>
  class Expression<Type> : public Resource<Type> {
   public:
    using ExprType = VarExpressionTag;
    using ValueType = Type;
    using Resource<Type>::Resource;
  };
}  // namespace reaction