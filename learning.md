# 第一章
## tuple 包装没有拷贝的类
std::tuple<MyClass> tup(obj);  // ❌ 这里要求拷贝构造

std::ref(x) 返回的是一个轻量对象 std::reference_wrapper<T>，它内部只是存了一个指针或引用 —— 它不会拷贝 x 本身。
```
    std::tuple<std::reference_wrapper<Args>...> m_args;
```
reference_wrapper 其实保存的是一个指针,所以获取值的时候要用get()

## weak_ptr

```
#include <iostream>
#include <memory>

int main()
{
    auto sp = std::make_shared<int>(42);
    std::weak_ptr<int> wp = sp;

    std::cout << "use_count = " << sp.use_count() << "\n"; // 1

    wp.reset();
    std::cout << "after wp.reset, use_count = " << sp.use_count() << "\n"; // 1 (没变)

    sp.reset();
    std::cout << "after sp.reset, use_count = " << sp.use_count() << "\n"; // 0 (对象销毁)

    // std::cout << *sp << std::endl; // 访问空指针，未定义行为
    // std::cout << *wp.lock() << std::endl; // 访问空指针，未定义行为
    if (sp == nullptr) {
        std::cout << "sp is now empty.\n"; // sp is now empty.
    } else {
        std::cout << "sp is not empty.\n";
    }
}

```
sp 空了，即 sp.get() == nullptr，sp.use_count() == 0。

sp2 还持有对象，sp2.use_count() == 1，对象还活着。
```
weak_ptr(const std::shared_ptr<T>& r) noexcept;
weak_ptr(std::shared_ptr<T>&& r) noexcept;

```
也就是说，它既能从 const& 构造（拷贝 shared_ptr），也能从 && 构造（移动 shared_ptr）。std::move(ptr) 的原因是 避免多余的引用计数操作，并且表达“我不再需要这份 shared_ptr，
只保留一个 weak_ptr”的语义。

### reset
weak_ptr::reset() 只是把 weak_ptr 自己清空，不会影响 shared_ptr 的引用计数，也不会导致对象销毁。我不再观察这份资源了。
## shared_ptr
```
#include <iostream>
#include <memory>

int main()
{
    auto sp = std::make_shared<int>(42);
    std::cout << "use_count = " << sp.use_count() << "\n"; // 1

    auto sp2 = sp;
    std::cout << "use_count = " << sp.use_count() << "\n"; // 2

    sp.reset();
    std::cout << "after reset sp.use_count = " << sp.use_count() << "\n"; // 0 (sp空)
    // std::cout << *sp << std::endl; // 访问空指针会导致未定义行为
    std::cout << "sp2 = " << *sp2 << "\n"; // 42 (sp2仍然有效)
    std::cout << "sp2.use_count = " << sp2.use_count() << "\n"; // 1 (sp2还活着)
}


```


## CTAD/函数模板
函数的万能引用是因为函数调用是一瞬间有效的,他的推导具有隔离性,而类则不同,会违背C++的基本原则:生成确定性原则
C++17 引入了 类模板实参推导 (CTAD, Class Template Argument Deduction)。
它允许你在写构造函数调用时不写模板参数，编译器会根据构造函数的参数自动推导出模板参数。故而可以React(ptr);

```
template <typename T>
struct Box {
    Box(T t) {}
};

int main() {
    Box b(42); // 类模板实参推导 (CTAD) 自动推断出模板参数,自动推导成 Box<int>
}
```
## EBO基类优化
空基类被继承会优化掉原本的1个字节(空类占用一个字节),而组合会占用指针字节

## unique_ptr作为成员
此时类的拷贝构造是delete的,其子类也不应该有拷贝

## 模板类的函数不依赖模板参数
由于不依赖模板参数,跟T无任何关系,所以他必须可用(属于非待决名,在编译时的语义分析阶段会立刻查找),必须用this->getValue();进行调用
两阶段查找和延迟实例化机制
- 待决名,无需知道类型是否完整
- 实例化,如果类型仍不完整报错

```mermaid
graph TD
  Start([开始]) --> Input[语法分析]
  Input --> Process[语义分析]
  Process --> Compute[非待决名]
  Process --> Error[模板实例化]
  Process --> temp[待决名]
  Compute --> End([结束])
  Error --> End
  temp-->End
```
加了this 就依赖了模板类型参数,属于待决名,

## 存在引用
就会有生命周期依赖的问题

# 第二章
## 观察者模式
观察者容器,注意我们定义的类型是Reaction的不同变参的类型,即每个类型都不同.

## 类型擦除
类型擦除是一种常见的编程技术，它允许你在保持类型安全的前提下，处理多种不同类型的数据，而无需在编译时知道这些类型的具体信息。

## 折叠表达式!=参数包展开
弃值表达式,使用副作用,逗号表示式左折叠和右折叠都是无影响的,因为逗号表达式自己有顺序,C++17使用std::initializer_list实现

## 类型萃取+别名传递
具备更强大的类型适配能力

## 数据源的区分
使用成员变量区分?那是运行时开销,故而模仿迭代器的类型,使用Tag进行类型区分.

# 第四章
## 标签分发
stl的迭代器用的此,比如tag

## 编译器反射
C++26才会有

## C++新特性
auto 用于变量声明时，必须有初始化表达式，编译器才能推断出具体类型。类的静态成员在类内只是声明，不能提供初始化器（除了 const static 整型），所以类内不能用 auto 声明静态成员

在类的静态成员变量声明中使用 auto 是不允许的，因为：

- 类内的静态成员声明只是声明，不是定义
- auto 需要初始化器才能推断类型
- 类内不能初始化非 constexpr 的静态成员

在 C++ 中，非静态成员变量也不能直接使用 auto 声明。

C++ 标准明确规定：类的非静态成员变量声明时，不能使用 auto 作为类型说明符，因为编译器无法在类定义阶段推断非静态成员的具体类型（非静态成员的初始化通常在构造函数中，而类定义时构造函数尚未执行）。

## weak_ptr

无论是拷贝还是移动构造，weak_ptr 都不会影响它所观察对象的引用计数
移动构造后，源 weak_ptr 变得无效（expired() 会返回 true）
拷贝构造后，两个 weak_ptr 相互独立，一个的重置不会影响另一个

## auto
模板函数的返回值是auto

## using

### 重用构造函数
```
// 基类
template <typename T, typename... Args>
struct Base {
    Base(int a) {}
    Base(double b, Args... args) {}
};

// 派生类：不使用using的写法
template <typename T, typename... Args>
struct Derived1 : Base<T, Args...> {
    // 必须手动转发每个构造函数
    Derived1(int a) : Base<T, Args...>(a) {}
    Derived1(double b, Args... args) : Base<T, Args...>(b, args...) {}
};

// 派生类：使用using的写法（更简洁）
template <typename T, typename... Args>
struct Derived2 : Base<T, Args...> {
    // 一次性继承基类所有构造函数
    using Base<T, Args...>::Base;
};

```
简化代码，避免重复定义
如果派生类需要继承基类的全部或部分构造函数，无需在派生类中手动编写构造函数并显式调用基类构造函数（如 Derived(...) : Base(...) {}），直接通过 using 声明即可继承，减少冗余代码。
保持接口一致性
当基类有多个构造函数时（如不同参数列表的重载），using 声明会一次性继承基类所有构造函数，确保派生类与基类的构造接口保持一致，避免遗漏某些构造方式。
支持参数转发
继承的基类构造函数会自动适配派生类的初始化需求，无需手动处理参数转发逻辑，尤其适合模板类或参数复杂的场景。
与派生类成员兼容
继承的基类构造函数会先初始化基类部分，再初始化派生类自己的成员变量（如果有），符合正常的对象构造顺序。
### 模板元
与模板元编程结合，定义条件类型
通过 std::conditional 等元函数，结合别名可定义 “条件类型”。
```
#include <type_traits>

// 根据T是否为整数，定义不同的类型
template <typename T>
using NumericType = std::conditional_t<
    std::is_integral_v<T>,
    long long,  // 若T是整数，用long long
    double      // 否则用double
>;

NumericType<int> a;    // 实际类型：long long
NumericType<float> b;  // 实际类型：double
```

```
template <typename T>
  struct ExpressionTraits {
    using Type = T;
  };

  template <typename T>
  struct ExpressionTraits<React<ReactImpl<T>>> {
    using Type = T;
  };
```
这种技术常用于泛型编程中处理类型包装的场景，例如：

- 剥离类型包装：当代码中存在多层封装的类型（如 React<ReactImpl<T>>），但实际需要操作的是内层的 T 时，用这种方式可以自动 “解包”。
- 统一接口适配：无论输入类型是原始类型还是包装类型，都能通过 ExpressionTraits<T>::Type 获得一致的底层类型，简化后续逻辑。
- 类型转换的钩子：可以根据需要添加更多特化版本，例如针对 React<OtherImpl<T>> 或 Wrapper<T> 等其他包装类型，定制不同的底层类型提取规则。


总结来说，这是 C++ 模板元编程中一种常见的 “类型萃取” 技巧，通过 using 定义的类型别名作为 “出口”，配合模板特化实现对不同类型的差异化处理，最终目的是从复杂类型中提取出需要的底层类型


# 编译期反射
## 入侵式的反射
类中定义宏
## 非入侵式的反射
aggregate类型和非聚合类型
### POD 
POD 是 C++ 为了兼容 C 语言而设计的概念 ——POD 类型的内存布局与 C 语言的结构体完全一致，且支持 C 风格的操作（如 memcpy、C 函数传参）。
一个类型是 POD，当且仅当它同时满足以下两个条件：

    是 Trivial 类型（确保无自定义生命周期逻辑，与 C 兼容）；
    是 Standard-layout 类型（确保内存布局与 C 一致）。

补充：Standard-layout 类型的判断条件
Standard-layout 是 “内存布局兼容 C” 的核心，需满足：

    没有 虚函数 或 虚基类（无虚表指针开销）；
    所有非静态成员的 访问控制一致（全 public、或全 private、或全 protected，不能混合）；
    没有 非 Standard-layout 的基类（基类也需是 Standard-layout）；
    没有 引用类型的非静态成员（引用不是 “数据”，是别名，C 中无对应概念）；
    （针对 union）若包含非 POD 成员，则整个 union 不是 Standard-layout。

C/C++ 混合编程：POD 类型可直接在 C 和 C++ 代码间传递（如函数参数、结构体成员），无需担心布局差异；
确保 内存操作的安全性：POD 类型用 memcpy 等操作完全安全，行为与 C 语言一致。


### Trivial
Trivial 类型是指所有 “特殊成员函数”（构造、析构、拷贝 / 移动赋值、拷贝 / 移动构造）均由编译器自动生成，且无自定义逻辑的类型。这类类型的 “生命周期管理” 完全由编译器接管，没有用户干预。
一个类型是 Trivial，当且仅当它满足以下所有条件：

    有 平凡的默认构造函数：即构造函数是编译器自动生成的（不允许用户自定义、= default 或 = delete？注意：C++11 后 = default 的默认构造函数也视为平凡的，只要没有其他自定义逻辑）；
    有 平凡的析构函数：无用户自定义析构函数（编译器自动生成），且析构函数不是虚函数（virtual）；
    有 平凡的拷贝构造函数 和 平凡的拷贝赋值运算符（均为编译器自动生成，无自定义）；
    有 平凡的移动构造函数 和 平凡的移动赋值运算符（均为编译器自动生成，无自定义）；
    不是 虚基类 或 包含虚基类（虚继承会引入非平凡逻辑）。

支持 高效的内存操作：如 memcpy、memset 等，因为 Trivial 类型的内存布局无额外逻辑（如虚表指针、自定义析构需要的状态），直接拷贝内存等价于 “逻辑拷贝”；
编译器可对 Trivial 类型的对象生命周期做优化（如省略不必要的构造 / 析构）。


### Aggregate
一个类型是 Aggregate，当且仅当它满足以下所有条件：

    是 class、struct 或 union（不包括 enum）；
    没有 用户声明的构造函数（允许编译器自动生成的默认构造函数，不允许 = default 或 = delete 显式声明的构造函数？注意：C++20 放松了这一点，允许 = default 的构造函数）；
    没有 private 或 protected 的非静态成员（成员访问控制必须是 public，union 除外，因为 union 成员默认共享访问）；
    没有 虚函数（virtual 函数会破坏简单布局）；
    没有 虚基类（虚继承会引入额外的虚表指针，破坏布局）；
    （C++17 新增）允许有 非虚基类（但基类也必须是 Aggregate，且初始化时需按基类在前、派生类成员在后的顺序）。

```
struct Base { int a; };
struct Derived : Base { int b; }; // Base 是 Aggregate，Derived 也是 Aggregate

int main() {
    Derived d = {{1}, 2}; // 合法（基类成员先初始化，派生类成员后初始化）
    return 0;
}
```

    支持 聚合初始化（Type obj = {成员1, 成员2, ...}），语法简洁，无需定义构造函数；
    编译器对 Aggregate 的内存布局有更简单的规则（成员按声明顺序排列，无额外开销如虚表指针）。


### Standard Layout 类型

## 非类型模板参数
比如常量,指针,枚举,成员指针等

## void类型不能做函数的参数,但是包装类可以

## maybeunused

## function 类型擦除
形参包放在lambda的捕获列表中==形参包放在了匿名lambda的成员变量中
实现维护实时存储一个动态的形参包

## string的移动构造,本身的reset为空