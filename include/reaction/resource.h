#include <exception>
#include <memory>
#include <stdexcept>

#include "observerNode.h"
namespace reaction {
  template <typename Type>
  class Resource : public ObserverNode {
   public:
    Resource() : m_ptr(nullptr) {}

    template <typename T>
    Resource(T&& t) : m_ptr(std::make_unique<Type>(std::forward<T>(t))) {}
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

    Resource(Resource&&) = default;
    Resource& operator=(Resource&&) = default;  // 禁止拷贝，允许移动

    Type& getValue() const {
      if (!m_ptr) {
        throw std::runtime_error("Resource is not initialized");
      }
      return *m_ptr;
    }

    Type* getRawPtr() const {
      if (!m_ptr) {
        throw std::runtime_error("Resource is not initialized");
      }
      return m_ptr.get();
    }

    template <typename T>
    void updateValue(T&& t) {
      if (!m_ptr) {
        m_ptr = std::make_unique<Type>(std::forward<T>(t));
      } else {
        *m_ptr = std::forward<T>(t);  // 否则移动构造会reset
      }
    }

   private:
    std::unique_ptr<Type> m_ptr;
  };

  struct VoidWrapper {};
  template <>
  class Resource<VoidWrapper> : public ObserverNode {
   public:
    // void 是无需创建unique_ptr
    auto getValue() const { return VoidWrapper{}; }
  };

}  // namespace reaction