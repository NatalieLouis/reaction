#include <exception>
#include <memory>

namespace reaction {
  template <typename Type>
  class Resource {
   public:
    Resource() : m_ptr(nullptr) {}

    template <typename T>
    Resource(T&& t) : m_ptr(std::make_unique<Type>(std::forward<T>(t))) {}
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

    Resource(Resource&&) = default;
    Resource& operator=(Resource&&) = default;

    Type& getValue() const {
      if (!m_ptr) {
        throw std::runtime_error("Resource is not initialized");
      }
      return *m_ptr;
    }
    void updateValue() {}

   private:
    std::unique_ptr<Type> m_ptr;
  };

}  // namespace reaction