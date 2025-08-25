
#include <iostream>
#include <string>
#include <tuple>

int global_a = 10;

template <typename T>
struct Field {};

struct Dog {
  bool m_male;
};

struct Person {
  Field<std::string> m_name;
  Field<int> m_age;
  bool m_male;
};

class PersonPrivate {
  Field<std::string> m_name;
  Field<int> m_age;
  bool m_male;
};

template <class T>
struct my_wrapper {
  inline static T value;
};

template <class T>
inline constexpr T& get_global_object() noexcept {
  return my_wrapper<T>::value;
}

template <auto val>
inline constexpr std::string_view getFunName() {
#if defined(__GNUC__)
  constexpr std::string_view func_name = __PRETTY_FUNCTION__;  // 内建宏
#elif defined(_MSC_VER)
  constexpr std::string_view func_name = __FUNCSIG__;
#endif

  size_t pos1 = func_name.find("val = ");
  if (pos1 == std::string_view::npos) {
    return {};
  }

  size_t pos2 = func_name.find(";", pos1 + 6);
  if (pos2 == std::string_view::npos) {
    return {};
  }
  return func_name.substr(pos1 + 6, pos2 - (pos1 + 6));  // 提取val的值
}

template <typename T>
struct Is_Field : std::false_type {};

template <typename T>
struct Is_Field<Field<T>> : std::true_type {};

template <typename Tuple>
constexpr bool check_field(const Tuple& tp) {
  bool found = false;
  std::apply(
      [&](auto&&... args) { ((found = found || Is_Field<std::decay_t<decltype(args)>>{}), ...); },
      tp);
  return found;
}

int main() {
  // int a = 10; 运行期的值不能作为非类型模板参数
  auto name = getFunName<&global_a>();  // 非类型参数必须是编译器确定的
  std::cout << name << std::endl;

  auto&& [m1, m2, m3] = get_global_object<Person>();  // 结构化绑定+转发引用
  constexpr auto tp = std::tie(m1, m2, m3);
  [&]<size_t... Is>(std::index_sequence<Is...>) {
    (std::cout << ... << getFunName<&std::get<Is>(tp)>());
    std::cout << std::endl;
  }(std::make_index_sequence<std::tuple_size_v<decltype(tp)>>{});

  /*
    Person&& temp = Person{}; // 转发引用绑定到临时对象（右值引用）
    auto& m1 = temp.name;    // 绑定到第一个成员
    auto& m2 = temp.age;     // 绑定到第二个成员
    auto& m3 = temp.height;  // 绑定到第三个成员
  */

  static_assert(check_field(tp), "No Field member found");  // 编译期断言
}
