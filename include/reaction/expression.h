#include <tuple>

#include "resource.h"
namespace reaction {
  template <typename Fun, typename... Args>
  class Expression : public Resource<> {
   public:
   private:
    Fun m_fun;
    std::tuple<Args...> m_args;
  };

  template <typename T>
  class Expression<T> : Resource<T> {};
}  // namespace reaction