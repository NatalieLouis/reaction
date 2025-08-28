
#include <cstddef>
#include <iostream>
#include <string>
#include <tuple>

int global_a = 10;

template <typename T>
struct Field {};

// 模板结构体，包含一个静态成员 value,有默认构造
template <class T>
struct my_wrapper {
  inline static T value;
};

template <typename T, auto... field>
struct private_visitor {
  friend constexpr auto get_private_ptrs(const my_wrapper<T>&) {
    constexpr auto tp = std::make_tuple(field...);
    return tp;
  }
};

// 声明友元函数并实例化模板,##会在__VA_ARGS__为空时去掉前面的逗号
#define REFL_PRIVATE(CLASS, ...)                                    \
  inline constexpr auto get_private_ptrs(const my_wrapper<CLASS>&); \
  template struct private_visitor<CLASS, ##__VA_ARGS__>;

struct Dog {
  bool m_male;
};

struct Person {
  Field<std::string> m_name;
  Field<int> m_age;
  bool m_male;
  Dog m_dog;
};

class PersonPrivate {
  Field<std::string> m_PrivateName;
  Field<int> m_PrivateAge;
  bool m_PrivateMale;
};

REFL_PRIVATE(PersonPrivate, &PersonPrivate::m_PrivateName, &PersonPrivate::m_PrivateAge,
             &PersonPrivate::m_PrivateMale)
// REFL_PRIVATE(&PersonPrivate2::m_PrivateName, &PersonPrivate2::m_PrivateAge,
//  &PersonPrivate2::m_PrivateMale) 会造成友元函数重定义,friend
//  函数写在结构体里，如果没有限定作用域，依然是全局函数

// 函数模板，用于获取 my_wrapper<T> 中定义的全局 value
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

struct AnyType {
  template <typename T>
  operator T();  // 转换为任意类型
};

template <typename T>
consteval size_t countMember(auto&&... Args) {
  if constexpr (!requires { T{Args...}; }) {
    return sizeof...(Args) - 1;
  } else {
    return countMember<T>(Args..., AnyType{});
  }
}

template <typename T>
constexpr size_t member_count_v = countMember<T>();

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
// template <typename T>
// constexpr auto getTuple() {
//   auto&& [m1, m2, m3] = get_global_object<T>();
//   return std::tie(m1, m2, m3);
// }

// template <typename T>
// constexpr bool reflectField() {
//   constexpr auto ref_tup = getTuple<T>();
//   return check_field(ref_tup);
// }

template <typename T, std::size_t n>
struct ReflectHelper {};

#define REF_STRUCT(n, ...)                           \
  template <typename T>                              \
  struct ReflectHelper<T, n> {                       \
    static constexpr auto getTuple() {               \
      auto&& [__VA_ARGS__] = get_global_object<T>(); \
      return std::tie(__VA_ARGS__);                  \
    }                                                \
    static constexpr auto reflectFieldImpl() {       \
      constexpr auto ref_tup = getTuple();           \
      return check_field(ref_tup);                   \
    }                                                \
  };

REF_STRUCT(1, m1)
REF_STRUCT(2, m1, m2)
REF_STRUCT(3, m1, m2, m3)
REF_STRUCT(4, m1, m2, m3, m4)
REF_STRUCT(5, m1, m2, m3, m4, m5)
REF_STRUCT(6, m1, m2, m3, m4, m5, m6)
REF_STRUCT(7, m1, m2, m3, m4, m5, m6, m7)

template <typename T>
constexpr auto reflectField(auto&&... Args) {
  return ReflectHelper<T, member_count_v<T>>::reflectFieldImpl();
}

int main() {
  // int a = 10; 运行期的值不能作为非类型模板参数
  auto name = getFunName<&global_a>();  // 非类型参数必须是编译器确定的
  std::cout << name << std::endl;

  auto&& [m1, m2, m3, m4] = get_global_object<Person>();  // 结构化绑定+转发引用
  constexpr auto tp = std::tie(m1, m2, m3, m4);

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
  static_assert(reflectField<Person>(), "Field member found");
  constexpr auto tp2 = ReflectHelper<Person, member_count_v<Person>>::getTuple();
  [&]<size_t... Is>(std::index_sequence<Is...>) {
    (std::cout << ... << getFunName<&std::get<Is>(tp2)>());
    std::cout << std::endl;
  }(std::make_index_sequence<std::tuple_size_v<decltype(tp2)>>{});

  std::cout << "===========================================" << std::endl;

  constexpr auto private_tp = get_private_ptrs(my_wrapper<PersonPrivate>{});
  [&]<size_t... Is>(std::index_sequence<Is...>) {
    (std::cout << ... << getFunName<std::get<Is>(private_tp)>());
    std::cout << std::endl;
  }(std::make_index_sequence<std::tuple_size_v<decltype(private_tp)>>{});
}
