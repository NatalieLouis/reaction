#include <atomic>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include "expression.h"
#include "reaction/observerNode.h"

namespace reaction {
  inline thread_local std::function<void(NodePtr)> g_reg_fun;

  struct RegGuard {
    RegGuard(const std::function<void(NodePtr)>& reg_fun) { g_reg_fun = reg_fun; }
    ~RegGuard() { g_reg_fun = nullptr; }
  };

  template <typename Type, typename... Args>
  class ReactImpl : public Expression<Type, Args...> {
   public:
    using Expression<Type, Args...>::Expression;  // 调用基类构造函数

    using ExprType = typename Expression<Type, Args...>::ExprType;
    using ValueType = typename Expression<Type, Args...>::ValueType;

    decltype(auto) get() const { return this->getValue(); }
    auto getRaw() const { return this->getRawPtr(); }

    template <typename F, HasArgs... A>
    void set(F&& fun, A&&... args) {
      this->setSource(std::forward<F>(fun), std::forward<A>(args)...);
    }

    template <typename F>
    void set(F&& fun) {
      // 数据源正在创建时,就进行赋值
      RegGuard guard([this](NodePtr node) { this->addObCb(node); });  // 避免有return无法nullptr
      this->setSource(std::forward<F>(fun));
    }

    void set() {
      RegGuard guard([this](NodePtr node) { this->addObCb(node); });  // 避免有return无法nullptr
      this->setOpExpr();
    }

    template <typename T>
      requires(ConvertCC<T, ValueType> && VarExprCC<VarExpressionTag> && !ConstType<ValueType>)
    void value(T&& t) {
      this->updateValue(std::forward<T>(t));  // 要求类型可转换，且不是const类型,且是var类型
      this->notify();
    }
    void addWeakRef() { ++m_weakRefCount; }  // 引用计数增加

    void removeWeakRef() {
      if (--m_weakRefCount == 0) {
        ObserverGraph::instance().removeNode(
            this->shared_from_this());  // 引用计数为0时，从观察图中移除节点
      }
    }

   private:
    std::atomic<int> m_weakRefCount{0};
  };

  template <typename ReactType>
  class React {
   public:
    using ValueType = typename ReactType::ValueType;

    // ReactImpl<std::decay_t<T>> == ReactType
    explicit React(std::shared_ptr<ReactType> ptr) : m_weakPtr(std::move(ptr)) {
      if (auto sharedPtr = m_weakPtr.lock()) {
        sharedPtr->addWeakRef();
      }
    }
    ~React() {
      if (auto sharedPtr = m_weakPtr.lock()) {
        sharedPtr->removeWeakRef();
      }
    }
    React(const React& other) : m_weakPtr(other.m_weakPtr) {
      if (auto sharedPtr = m_weakPtr.lock()) {
        sharedPtr->addWeakRef();
      }
    }
    React(React&& other) noexcept : m_weakPtr(std::move(other.m_weakPtr)) {
      other.m_weakPtr.reset();
    }
    React& operator=(const React& other) {
      if (this != &other) {
        if (auto sharedPtr = m_weakPtr.lock()) {
          sharedPtr->removeWeakRef();
        }
        m_weakPtr = other.m_weakPtr;
        if (auto sharedPtr = m_weakPtr.lock()) {
          sharedPtr->addWeakRef();
        }
      }
      return *this;
    }
    React& operator=(React&& other) noexcept {
      if (this != &other) {
        if (auto sharedPtr = m_weakPtr.lock()) {
          sharedPtr->removeWeakRef();
        }
        m_weakPtr = std::move(other.m_weakPtr);
        other.m_weakPtr.reset();
      }
      return *this;
    }
    auto operator->() const { return getSharedPtr()->getRaw(); }

    explicit operator bool() const { return !m_weakPtr.expired(); }

    decltype(auto) operator()() const {
      if (g_reg_fun) {
        std::invoke(g_reg_fun, this->getSharedPtr());
      }
      return get();
    }

    decltype(auto) get() const
    // requires IsDataReact<ReactType>
    {
      return getSharedPtr()->get();  // Impl的get 获取resource的值,需要是有值的才能有get
    }

    template <typename F, typename... A>
    void reset(F&& t, A&&... args) {
      return getSharedPtr()->set(std::forward<F>(t), std::forward<A>(args)...);
    }
    // 更新值
    template <typename T>
    void value(T&& t) {
      getSharedPtr()->value(std::forward<T>(t));
    }

    std::shared_ptr<ReactType> getSharedPtr() const {
      auto sharedPtr = m_weakPtr.lock();
      if (!sharedPtr) {
        throw std::runtime_error("Attempt to access a moved-from React object.");
      }
      return sharedPtr;
    }

   private:
    std::weak_ptr<ReactType> m_weakPtr;
  };

  template <typename T>
  auto var(T&& t) {
    auto ptr = std::make_shared<ReactImpl<std::decay_t<T>>>(std::forward<T>(t));
    ObserverGraph::instance().addNode(ptr);  // 将节点加入观察图
    return React(ptr);
  }

  template <typename T>
  auto constVar(T&& t) {
    auto ptr = std::make_shared<ReactImpl<const std::decay_t<T>>>(std::forward<T>(t));
    ObserverGraph::instance().addNode(ptr);
    return React(ptr);
  }

  template <typename Func, typename... Args>
  auto calc(Func&& fun, Args&&... args) {
    auto ptr = std::make_shared<ReactImpl<std::decay_t<Func>, std::decay_t<Args>...>>();
    ObserverGraph::instance().addNode(ptr);
    ptr->set(std::forward<Func>(fun), std::forward<Args>(args)...);
    return React(ptr);
  }

  template <typename OpExpr>
  auto expr(OpExpr&& opExpr) {
    auto ptr = std::make_shared<ReactImpl<std::decay_t<OpExpr>>>(std::forward<OpExpr>(opExpr));
    ObserverGraph::instance().addNode(ptr);
    ptr->set();
    return React(ptr);
  }

  template <typename Func, typename... Args>
  auto action(Func&& fun, Args&&... args) {
    return calc(std::forward<Func>(fun), std::forward<Args>(args)...);
  }

}  // namespace reaction