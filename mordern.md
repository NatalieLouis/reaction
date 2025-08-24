# default 
```
#include <iostream>
class A {
public:
    A(int a)
        : num(a) {
        std::cout << "A constructed\n";
    }
    ~A() { std::cout << "A destructed\n"; }
    int num = 8;
};
class B {
public:
    B() = default;
    B(int a)
        : obj(a) { }
    A obj;
    int num = 5;
};

int main() {
    B b;
    std::cout << "B's A num: " << b.obj.num << "\n";
    return 0;
}
```

和
```
#include <iostream>
class A {
public:
    A(int a)
        : num(a) {
        std::cout << "A constructed\n";
    }
    ~A() { std::cout << "A destructed\n"; }
    int num = 8;
};
class B {
public:
    B()
        : obj(3) { }
    A obj;
    int num = 5;
};

int main() {
    B b;
    std::cout << "B's A num: " << b.obj.num << "\n";
    return 0;
}
```
换言之类里面的只是声明,当构造的时候才是定义.A obj; 作为类 B 的成员变量声明时没有直接报错，是因为成员变量的声明本身不要求立即初始化，真正的初始化发生在类 B 的构造函数执行阶段。

# 构造顺序
多继承的构造顺序核心是 “声明顺序优先”，具体牢记：

    虚基类最先（最终派生类声明顺序，只一次）；
    非虚基类按派生类中声明顺序（与初始化列表无关）；
    成员对象按类中声明顺序（与初始化列表无关）；
    派生类自身最后。
```
#include <iostream>
using namespace std;

// 1. 基础类（无父类）
class X {
public:
    X() { cout << "X 构造\n"; }
    virtual ~X() { cout << "X 析构\n"; }
};

class Y {
public:
    Y() { cout << "Y 构造\n"; }
    virtual ~Y() { cout << "Y 析构\n"; }
};

// 2. 虚基类（继承自X）
class A : virtual public X { // 虚继承X
public:
    A() { cout << "A 构造\n"; }
    virtual ~A() { cout << "A 析构\n"; }
};

// 3. 非虚基类（继承自Y）
class B : public Y { // 非虚继承Y
public:
    B() { cout << "B 构造\n"; }
    virtual ~B() { cout << "B 析构\n"; }
};

// 4. 多层继承类（继承A和B，声明顺序A→B）
class C : public A, public B { // 基类声明顺序：A在前，B在后
public:
    C() : B(), A() { cout << "C 构造\n"; }
    virtual ~C() { cout << "C 析构\n"; }
};

// 5. 成员对象类（继承B和A，声明顺序B→A）
class D : public B, public A { // 基类声明顺序：B在前，A在后
public:
    D() { cout << "D 构造\n"; }
    virtual ~D() { cout << "D 析构\n"; }
};

// 6. 最终派生类（继承C，包含成员对象D和A）
class E : public C { // 继承C（C本身继承A、B）
private:
    D d;
    A a;

public:
    E() : a(), d(), C() { cout << "E 构造\n"; }
    virtual ~E() { cout << "E 析构\n"; }
};

int main() {
    cout << "创建E对象时的构造顺序：\n";
    E e;
    return 0;
}
```
要确定E对象的占用字节数，需要结合 C++ 对象的内存布局规则，主要考虑以下因素：虚函数表指针（vptr）、虚基类指针（vbptr）、继承关系的成员布局以及内存对齐（假设为 64 位系统，指针占 8 字节，对齐单位为 8 字节）。
关键分析步骤：

    虚函数表指针（vptr）
    所有包含虚函数（或继承自含虚函数的类）的类，其对象会包含一个vptr（指向虚函数表），占 8 字节。
    虚基类的额外开销（vbptr）
    虚继承（如A : virtual public X）会引入虚基类指针（vbptr），用于定位共享的虚基类子对象，占 8 字节。
    各相关类的内存大小
    逐个分析涉及的类（无成员变量，仅考虑 vptr 和 vbptr）：
        X：含虚析构函数 → 1 个 vptr → 8 字节。
        Y：含虚析构函数 → 1 个 vptr → 8 字节。
        A：虚继承 X，自身含虚函数 → A 的 vptr（8） + 虚基类指针 vbptr（8） + 虚基类 X 的 vptr（8） → 共 24 字节。
        B：非虚继承 Y，自身含虚函数 → 继承 Y 的 vptr（B 的 vptr 与 Y 的 vptr 合并） → 8 字节。
        C：继承 A 和 B（顺序 A→B） → A 的大小（24） + B 的大小（8） → 共 32 字节。
        D：继承 B 和 A（顺序 B→A） → B 的大小（8） + A 的大小（24） → 共 32 字节。
    E 对象的组成
    E的结构：
        继承自C → 32 字节（C 的大小）。
        成员对象D d → 32 字节（D 的大小）。
        成员对象A a → 24 字节（A 的大小）。
    总大小：32（C） + 32（d） + 24（a） = 88 字节。
    内存对齐验证
    64 位系统下对齐单位为 8 字节，88 是 8 的整数倍（88 ÷ 8 = 11），无需额外填充。

结论：
在 64 位系统下，E对象的大小为 88 字节。
# auto 和 auto&

auto：推导出的是非引用类型，本质是 “值类型”。当用 auto 声明变量时，编译器会根据初始化表达式的类型，推导出一个 “值类型”，并拷贝初始化表达式的值（如果表达式是对象）。
auto&：推导出的是引用类型，本质是 “绑定到初始化表达式的引用”。它不会拷贝对象，而是直接绑定到原对象，相当于给原对象起了一个 “别名”。auto& 不会舍弃cv(const和引用)
```
const int y = 5;
auto c = y;       // c 是 int 类型（非 const，因为拷贝了 y 的值）
c = 10;           // 合法（c 本身不是常量）

auto& d = y;      // d 是 const int& 类型（引用必须与原对象的 const 性一致）
d = 10;           // 编译错误（不能修改 const 引用指向的值）
```

```
int x = 10;
int& ref_x = x;
auto a = ref_x;  // a 是 int 类型（拷贝 ref_x 指向的值，即 x 的值）
```

```
const int y = 5;
auto& d = y;      // d 是 const int& 类型（引用必须与原对象的 const 性一致）
d = 10;           // 编译错误（不能修改 const 引用指向的值）

int z = 8;
const auto& e = z;  // e 是 const int&，绑定 z 但不能修改
e = 9;              // 编译错误

auto& f = 10;       // 错误：10 是右值，不能绑定到非 const 引用
const auto& g = 10; // 正确：const 引用可以绑定到右值（临时对象生命周期被延长）

```

在 C++ 中，引用一旦初始化绑定到某个对象后，就不能再更改其绑定的目标（引用的 “从一而终” 特性

# 库
在 C++ 库封装中，核心目标是只暴露必要的接口，隐藏内部实现细节—— 包括内部头文件。用户提示 “内部头文件缺失”，通常是因为你的库设计中 “公开接口” 不小心依赖了内部头文件，导致使用者必须拿到这些内部文件才能编译。解决这个问题的关键是通过合理的接口设计和前置声明（forward declaration） 等技巧，切断公开接口对内部头文件的依赖。
## 前置声明（forward declaration)
减少公开头文件依赖的核心技巧,前置声明的作用是：在不包含头文件的情况下，告诉编译器 “这个类型存在”，从而避免在公开接口中引入不必要的头文件（尤其是内部头文件）。
适用场景：当公开接口中只用到某个类型的 “指针 / 引用” 时
如果你的公开类 / 函数的接口中，只涉及某个类型的指针或引用（而非具体成员访问、对象创建），就可以用前置声明代替 #include。
```
// 正确做法：用前置声明代替#include
// public.h（公开头文件）
class InternalClass;  // 前置声明：告诉编译器这个类型存在，无需知道细节

class PublicClass {
public:
    // 只用到指针，前置声明足够
    void setInternal(InternalClass* ptr); 
};

```
此时，public.h 不再依赖 internal.h，使用者只需 public.h 即可编译，无需知道 InternalClass 的具体定义（因为链接时库文件已经包含了它的实现）。
## PImpl
如果公开类的内部实现依赖很多内部类型，前置声明可能不够用（比如需要在公开类中定义内部类型的成员变量）。这时可以用 Pimpl 模式（Pointer to Implementation），把所有实现细节隐藏到一个 “内部实现类” 中，公开类只持有它的指针。

```
// public.h（公开头文件，仅发布这个）
#include <memory>  // 智能指针需要

// 前置声明：内部实现类（使用者无需知道它的定义）
class PublicClassImpl;

class PublicClass {
public:
    PublicClass();
    ~PublicClass();  // 必须在cpp中实现，否则智能指针析构会报错
    void doSomething();  // 公开接口
private:
    // 指向内部实现的智能指针（隐藏所有成员变量）
    std::unique_ptr<PublicClassImpl> impl;
};
```
解决这些问题的原则：公开接口中，对内部类型的使用只能是 “指针” 或 “引用”，且通过前置声明声明这些类型，永远不要在公开头文件中出现内部类型的完整定义或 #include。
如果某个结构体 / 枚举仅在实现中使用，直接在 .cpp 中定义，而非头文件：
```
// 正确：.cpp中定义
// public.cpp
enum InternalState { Init, Running, Done };  // 仅实现可见
```

## 模板显示实例化
编译 main.cpp 时：编译器只看到 MyTemplate<int>::func() 的声明，没有实现，无法生成该函数的具体代码（只知道 “有这个函数”，但不知道 “怎么实现”）；编译 my_template.cpp 时：编译器看到了实现，但由于没有 “显式告诉它需要实例化 MyTemplate<int>”，它不会主动生成 MyTemplate<int> 的代码（模板不会为 “所有可能的类型” 提前生成代码，那样太浪费）；链接阶段：main.obj 中引用了 MyTemplate<int>::func()，但所有目标文件中都没有这个函数的实现（没生成），导致 链接错误（undefined reference）。
如果确实想把模板实现放在 .cpp 中（比如隐藏实现细节），可以用显式实例化：在模板实现的 .cpp 中，主动告诉编译器 “需要为哪些类型生成代码”。

## 类型擦除
### 基类指针
### function
std::function：可存储函数、lambda、函数对象等，只要签名匹配（如 std::function<void(int)> 可存储任何 “接收 int、返回 void” 的可调用对象
### 静态擦除
基于模板的静态擦除（编译期擦除）
通过模板重载或特化，在编译期为不同类型生成适配代码，对外暴露统一接口（无需运行时多态）。例如标准库的 std::for_each 就是通过模板擦除容器的具体类型，只要容器支持迭代器即可。


C++ 的编译模型是 “单文件独立编译”，每个编译单元看不到其他单元的代码；
模板的实例化发生在编译期，且需要完整实现才能生成具体类型的代码；
若实现放在 .cpp 中，使用模板的编译单元在编译时看不到实现，无法实例化；而实现所在的 .cpp 也不会主动生成所有可能的实例，最终导致链接失败。

## 常见
## std::decay_t
## std::reference_wrapper
## std::invoke
C++17 引入的通用调用工具，可统一调用普通函数、成员函数、函数对象、lambda 等任何可调用对象（m_fun）。
## std::tuple
## std::remove_reference_t
## std::is_void_v
## std::invoke_result_t
## 类型别名

# 模板
立即调用的模板 lambda 表达式（IIFE，Immediately Invoked Function Expression）