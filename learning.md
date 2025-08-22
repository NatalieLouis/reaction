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