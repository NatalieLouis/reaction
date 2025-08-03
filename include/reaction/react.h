#include <atomic>
#include <memory>
#include <type_traits>
#include <utility>

#include "expression.h"
#include "reaction/observerNode.h"

namespace reaction {
  template <typename Type, typename... Args>
  class ReactImpl : public Expression<Type, Args...> {
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
    void addWeakRef() { ++m_weakRefCount; }

    void removeWeakRef() {
      if (--m_weakRefCount == 0) {
        ObserverGraph::instance().removeNode(this->shared_from_this());
      }
    }

   private:
    std::atomic<int> m_weakRefCount{0};
  };

  template <typename ReactType>
  class React {
   public:
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

    explicit operator bool() const { return !m_weakPtr.expired(); }

    auto get() const { return getSharedPtr()->get(); }

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
    ObserverGraph::instance().addNode(ptr);
    return React(ptr);
  }

  template <typename Func, typename... Args>
  auto calc(Func&& fun, Args&&... args) {
    auto ptr = std::make_shared<ReactImpl<std::decay_t<Func>, std::decay_t<Args>...>>(
        std::forward<Func>(fun), std::forward<Args>(args)...);
    ObserverGraph::instance().addNode(ptr);
    return React(ptr);
  }

}  // namespace reaction