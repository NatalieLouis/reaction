#include <concepts>
#include <memory>
#include <type_traits>

#include "reaction/observerNode.h"
namespace reaction {
  //==================================forward declaration=========================
  template <typename T, typename... Args>
  class ReactImpl;

  template <typename T>
  class React;

  template <typename Op, typename L, typename R>
  class BinaryOpExpr;

  template <typename T>
  struct ValueWrapper;

  struct VarExpressionTag;
  class ObserverNode;
  struct VoidWrapper;
  class FieldBase;

  using NodePtr = std::shared_ptr<ObserverNode>;

  //==================================concepts=========================================
  template <typename T, typename U>
  concept ConvertCC = std::is_convertible_v<std::decay_t<T>, std::decay_t<U>>;

  template <typename T>
  concept VarExprCC = std::is_same_v<std::decay_t<T>, VarExpressionTag>;

  template <typename T>
  concept ConstType = std::is_const_v<std::remove_reference_t<T>>;

  template <typename T>
  concept VoidType = std::is_void_v<T> || std::is_same_v<std::decay_t<T>, VoidWrapper>;

  template <typename T>
  concept InvocableType = std::is_invocable_v<std::decay_t<T>>;

  template <typename T>
  concept NonInvocableType = !InvocableType<T>;

  template <typename... Args>
  concept HasArgs = sizeof...(Args) > 0;

  template <typename T>
  concept IsReactNode = requires(T t) {
    { t.shared_from_this() } -> std::same_as<NodePtr>;
  };

  template <typename T>
  concept IsDataReact = requires(T t) {
    typename T::ValueType;  // 要求T类型中有ValueType别名
    requires(IsReactNode<T> && !VoidType<typename T::ValueType>);
  };

  //=====================expression traits=================================

  template <typename T>
  struct IsReact : std::false_type {};

  template <typename T>
  struct IsReact<React<T>> : std::true_type {
    using type = T;
  };

  template <typename T>
  struct ExpressionTraits {
    using type = T;
  };

  template <NonInvocableType T>
  struct ExpressionTraits<React<ReactImpl<T>>> {
    using type = T;
  };

  template <typename Fun, typename... Args>
  struct ExpressionTraits<React<ReactImpl<Fun, Args...>>> {
    using rawtype = std::invoke_result_t<Fun, typename ExpressionTraits<Args>::type...>;
    using type = std::conditional_t<VoidType<rawtype>, VoidWrapper, rawtype>;
  };

  template <typename Fun, typename... Args>
  using ExpressionType =
      typename ExpressionTraits<React<ReactImpl<Fun, Args...>>>::type;  // 避免typename来避免歧义

  template <typename T>
  struct BinaryOpExprTraits : std::false_type {};

  template <typename Op, typename L, typename R>
  struct BinaryOpExprTraits<BinaryOpExpr<Op, L, R>> : std::true_type {};

  template <typename T>
  concept IsBinaryOpExpr = BinaryOpExprTraits<std::decay_t<T>>::value;

  template <typename T>
  using ExprWrapper =
      std::conditional_t<IsReact<T>::value || IsBinaryOpExpr<T>, T, ValueWrapper<T>>;

  template <typename L, typename R>
  concept HasCustomOp = IsReact<std::decay_t<L>>::value || IsReact<std::decay_t<R>>::value ||
                        IsBinaryOpExpr<std::decay_t<L>> || IsBinaryOpExpr<std::decay_t<R>>;
}  // namespace reaction