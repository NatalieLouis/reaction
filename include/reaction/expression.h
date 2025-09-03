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

  template <typename Op, typename L, typename R>
  class BinaryOpExpr {
   public:
    using ValueType = typename std::common_type_t<typename L::ValueType, typename R::ValueType>;

    template <typename Left, typename Right>
    BinaryOpExpr(Left&& l, Right&& r, Op op = Op{})
        : m_l(std::forward<Left>(l)), m_r(std::forward<Right>(r)), m_op(op) {}

    auto operator()() const { return calculate(); }

   private:
    auto calculate() const { return m_op(m_l(), m_r()); }

   private:
    L m_l;
    R m_r;
    [[no_unique_address]] Op m_op;  // 空基类优化 1byte-->0 byte
  };

  struct AddOp {
    auto operator()(auto&& a, auto&& b) const { return a + b; }
  };
  struct SubOp {
    auto operator()(auto&& a, auto&& b) const { return a - b; }
  };
  struct MulOp {
    auto operator()(auto&& a, auto&& b) const { return a * b; }
  };
  struct DivOp {
    auto operator()(auto&& a, auto&& b) const { return a / b; }
  };

  template <typename Type>
  struct ValueWrapper {
    using ValueType = Type;
    ValueType m_value;

    template <typename T>
    ValueWrapper(T&& t) : m_value(std::forward<T>(t)) {}

    const ValueType& operator()() const { return m_value; }
    operator Type() const { return m_value; }  // 隐式转换
  };

  template <typename Op, typename L, typename R>
  auto make_binary_expr(L&& l, R&& r) {
    return BinaryOpExpr<Op, ExprWrapper<std::decay_t<L>>, ExprWrapper<std::decay_t<R>>>(
        std::forward<L>(l), std::forward<R>(r));
  }

  template <typename L, typename R>
    requires(HasCustomOp<L, R>)
  auto operator+(L&& l, R&& r) {
    return make_binary_expr<AddOp>(std::forward<L>(l), std::forward<R>(r));
  }
  template <typename L, typename R>
    requires(HasCustomOp<L, R>)
  auto operator-(L&& l, R&& r) {
    return make_binary_expr<SubOp>(std::forward<L>(l), std::forward<R>(r));
  }
  template <typename L, typename R>
    requires(HasCustomOp<L, R>)
  auto operator*(L&& l, R&& r) {
    return make_binary_expr<MulOp>(std::forward<L>(l), std::forward<R>(r));
  }

  template <typename L, typename R>
    requires(HasCustomOp<L, R>)
  auto operator/(L&& l, R&& r) {
    return make_binary_expr<DivOp>(std::forward<L>(l), std::forward<R>(r));
  }

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

  template <typename Op, typename L, typename R>
  class Expression<BinaryOpExpr<Op, L, R>>
      : public Expression<std::function<
            typename std::common_type_t<typename L::ValueType, typename R::ValueType>()>> {
   public:
    template <typename T>
    Expression(T&& t) : m_expr(std::forward<T>(t)) {}

   protected:
    void setOpExpr() {
      this->setSource([this]() { return m_expr(); });
    }

   private:
    BinaryOpExpr<Op, L, R> m_expr;
  };
}  // namespace reaction