# C++ 标准版本特性更新汇总

## 目录
- [C++11 (2011)](#c11-2011)
- [C++14 (2014)](#c14-2014)
- [C++17 (2017)](#c17-2017)
- [C++20 (2020)](#c20-2020)
- [特性对比表](#特性对比表)

---

## C++11 (2011)
> **现代C++的开始** - 这是C++历史上最重要的更新，引入了大量现代化特性

### 🚀 核心语言特性

#### 1. **auto 关键字**
```cpp
// 自动类型推导
auto x = 42;                    // int
auto y = 3.14;                  // double
auto z = std::vector<int>{1,2,3}; // std::vector<int>

// 复杂类型推导
std::map<std::string, std::vector<int>> data;
auto it = data.begin();         // 自动推导迭代器类型
```

#### 2. **decltype 关键字**
```cpp
int x = 10;
decltype(x) y = 20;             // y的类型是int

template<typename T, typename U>
auto add(T t, U u) -> decltype(t + u) {  // 返回类型推导
    return t + u;
}
```

#### 3. **范围for循环 (Range-based for)**
```cpp
std::vector<int> vec{1, 2, 3, 4, 5};

// 传统方式
for (size_t i = 0; i < vec.size(); ++i) {
    std::cout << vec[i] << " ";
}

// 现代方式
for (const auto& element : vec) {
    std::cout << element << " ";
}

// 修改元素
for (auto& element : vec) {
    element *= 2;
}
```

#### 4. **lambda 表达式**
```cpp
// 基本语法: [capture](parameters) -> return_type { body }
auto lambda = [](int x, int y) { return x + y; };
int result = lambda(5, 3);      // result = 8

// 捕获变量
int a = 10, b = 20;
auto capture_by_value = [a, b](int x) { return a + b + x; };
auto capture_by_ref = [&a, &b](int x) { a += x; b += x; };
auto capture_all_by_value = [=](int x) { return a + b + x; };
auto capture_all_by_ref = [&](int x) { a += x; b += x; };

// 在STL算法中使用
std::vector<int> vec{5, 2, 8, 1, 9};
std::sort(vec.begin(), vec.end(), [](int a, int b) { return a < b; });
```

#### 5. **右值引用和移动语义**
```cpp
class MyClass {
private:
    int* data;
    size_t size;
public:
    // 移动构造函数
    MyClass(MyClass&& other) noexcept 
        : data(other.data), size(other.size) {
        other.data = nullptr;
        other.size = 0;
    }
    
    // 移动赋值操作符
    MyClass& operator=(MyClass&& other) noexcept {
        if (this != &other) {
            delete[] data;
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0;
        }
        return *this;
    }
};

// 完美转发
template<typename T>
void forward_func(T&& t) {
    some_function(std::forward<T>(t));
}
```

#### 6. **智能指针**
```cpp
#include <memory>

// unique_ptr - 独占所有权
std::unique_ptr<int> ptr1 = std::make_unique<int>(42);
std::unique_ptr<int> ptr2 = std::move(ptr1);  // 转移所有权

// shared_ptr - 共享所有权
std::shared_ptr<int> sptr1 = std::make_shared<int>(42);
std::shared_ptr<int> sptr2 = sptr1;           // 引用计数+1

// weak_ptr - 弱引用，解决循环引用
std::weak_ptr<int> wptr = sptr1;
if (auto locked = wptr.lock()) {              // 检查对象是否存在
    // 使用locked指针
}
```

#### 7. **nullptr**
```cpp
// 替代NULL
void func(int* ptr) { /* ... */ }
void func(int value) { /* ... */ }

func(NULL);     // 模糊，可能调用func(int)
func(nullptr);  // 明确调用func(int*)
```

#### 8. **强类型枚举 (enum class)**
```cpp
// 传统枚举的问题
enum Color { RED, GREEN, BLUE };
enum Fruit { APPLE, ORANGE };
// int x = RED;  // 隐式转换为int
// if (RED == APPLE) // 意外比较

// 强类型枚举
enum class Color : int { RED, GREEN, BLUE };
enum class Fruit : int { APPLE, ORANGE };

Color c = Color::RED;
// int x = Color::RED;              // 错误，不能隐式转换
// if (Color::RED == Fruit::APPLE)  // 错误，不能比较不同枚举
```

#### 9. **统一初始化语法**
```cpp
// 大括号初始化
int x{42};
std::vector<int> vec{1, 2, 3, 4, 5};
std::map<std::string, int> map{{"one", 1}, {"two", 2}};

class Point {
public:
    int x, y;
    Point(int x, int y) : x(x), y(y) {}
};
Point p{10, 20};

// 防止窄化转换
int x = 3.14;   // 允许，但会截断
// int y{3.14}; // 错误，不允许窄化
```

#### 10. **委托构造函数**
```cpp
class MyClass {
private:
    int value;
    std::string name;
public:
    MyClass(int v, const std::string& n) : value(v), name(n) {}
    MyClass(int v) : MyClass(v, "default") {}      // 委托构造
    MyClass() : MyClass(0) {}                      // 委托构造
};
```

#### 11. **final 和 override 关键字**
```cpp
class Base {
public:
    virtual void func() {}
    virtual void final_func() final {}  // 不能被重写
};

class Derived : public Base {
public:
    void func() override {}             // 明确标记重写
    // void final_func() override {}    // 错误，不能重写final函数
};

class Final final : public Base {};     // 不能被继承
// class MoreDerived : public Final {}; // 错误
```

#### 12. **default 和 delete**
```cpp
class MyClass {
public:
    MyClass() = default;                    // 使用默认实现
    MyClass(const MyClass&) = delete;      // 禁用拷贝构造
    MyClass& operator=(const MyClass&) = delete; // 禁用拷贝赋值
    
    // 也可以删除特定重载
    void func(int) {}
    void func(double) = delete;             // 禁用double版本
};
```

### 📚 标准库新特性

#### 1. **新容器**
```cpp
// std::array - 固定大小数组
std::array<int, 5> arr{1, 2, 3, 4, 5};

// std::unordered_map/set - 哈希表
std::unordered_map<std::string, int> hash_map;
std::unordered_set<int> hash_set;

// std::forward_list - 单向链表
std::forward_list<int> flist{1, 2, 3};
```

#### 2. **线程支持库**
```cpp
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

// 线程
std::thread t1([]() { std::cout << "Hello from thread!\n"; });
t1.join();

// 互斥锁
std::mutex mtx;
std::lock_guard<std::mutex> lock(mtx);

// 原子操作
std::atomic<int> counter{0};
counter++;
```

#### 3. **正则表达式**
```cpp
#include <regex>

std::string text = "Hello 123 World 456";
std::regex pattern(R"(\d+)");  // 原始字符串字面量

std::sregex_iterator begin(text.begin(), text.end(), pattern);
std::sregex_iterator end;
for (auto it = begin; it != end; ++it) {
    std::cout << it->str() << std::endl;  // 输出: 123, 456
}
```

#### 4. **随机数库**
```cpp
#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(1, 100);

int random_num = dis(gen);
```

#### 5. **时间库 chrono**
```cpp
#include <chrono>

auto start = std::chrono::high_resolution_clock::now();
// 执行一些操作
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
std::cout << "Elapsed: " << duration.count() << "ms" << std::endl;
```

---

## C++14 (2014)
> **C++11的完善** - 相对较小的更新，主要完善C++11的特性

### 🔧 语言特性

#### 1. **泛型lambda**
```cpp
// C++11: 只能使用具体类型
auto lambda11 = [](int x) { return x * 2; };

// C++14: 可以使用auto参数
auto lambda14 = [](auto x) { return x * 2; };
lambda14(5);      // int
lambda14(5.5);    // double
lambda14("hi");   // 错误，字符串不支持*操作
```

#### 2. **返回类型推导**
```cpp
// C++11: 需要显式指定返回类型
auto func11(int x) -> int { return x * 2; }

// C++14: 可以省略返回类型
auto func14(int x) { return x * 2; }  // 自动推导为int

// 复杂情况
auto fibonacci(int n) {
    if (n <= 1) return n;             // int
    return fibonacci(n-1) + fibonacci(n-2);  // 也是int
}
```

#### 3. **变量模板**
```cpp
template<typename T>
constexpr T pi = T(3.1415926535897932385);

// 使用
double d = pi<double>;
float f = pi<float>;
```

#### 4. **广义捕获 (Generalized capture)**
```cpp
// 移动捕获
auto func = [ptr = std::make_unique<int>(42)](){ 
    return *ptr; 
};

// 表达式捕获
int x = 10;
auto lambda = [y = x + 1](int z) { return y + z; };
```

### 📚 标准库新特性

#### 1. **std::make_unique**
```cpp
// C++11: 只有make_shared
auto ptr11 = std::shared_ptr<int>(new int(42));

// C++14: 添加了make_unique
auto ptr14 = std::make_unique<int>(42);
```

#### 2. **用户定义字面量标准库支持**
```cpp
#include <chrono>
using namespace std::chrono_literals;

auto duration = 100ms;           // std::chrono::milliseconds
auto time_point = 1h + 30min;    // 1小时30分钟
```

#### 3. **std::integer_sequence**
```cpp
template<std::size_t... Is>
void print_sequence(std::index_sequence<Is...>) {
    ((std::cout << Is << " "), ...);  // C++17 fold expression
}

print_sequence(std::make_index_sequence<5>{});  // 输出: 0 1 2 3 4
```

---

## C++17 (2017)
> **重要更新** - 引入了很多实用特性，显著改善了C++编程体验

### 🚀 核心语言特性

#### 1. **结构化绑定 (Structured bindings)**
```cpp
// std::pair
std::pair<int, std::string> data{42, "hello"};
auto [number, text] = data;  // number=42, text="hello"

// std::tuple
std::tuple<int, double, std::string> tuple_data{1, 2.5, "world"};
auto [a, b, c] = tuple_data;

// 自定义结构
struct Point { int x, y; };
Point p{10, 20};
auto [x, y] = p;

// 在循环中使用
std::map<std::string, int> map{{"one", 1}, {"two", 2}};
for (const auto& [key, value] : map) {
    std::cout << key << ": " << value << std::endl;
}
```

#### 2. **if constexpr**
```cpp
template<typename T>
void process(T value) {
    if constexpr (std::is_integral_v<T>) {
        // 只有当T是整数类型时才编译这部分
        std::cout << "Processing integer: " << value << std::endl;
    } else if constexpr (std::is_floating_point_v<T>) {
        // 只有当T是浮点类型时才编译这部分
        std::cout << "Processing float: " << value << std::endl;
    } else {
        // 其他类型
        std::cout << "Processing other type" << std::endl;
    }
}
```

#### 3. **折叠表达式 (Fold expressions)**
```cpp
// 可变参数模板的简化
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);  // 右折叠: arg1 + (arg2 + (arg3 + ...))
}

template<typename... Args>
void print(Args... args) {
    ((std::cout << args << " "), ...);  // 左折叠
    std::cout << std::endl;
}

int result = sum(1, 2, 3, 4, 5);  // result = 15
print("Hello", 42, 3.14);         // 输出: Hello 42 3.14
```

#### 4. **类模板参数推导 (CTAD)**
```cpp
// C++14及之前
std::pair<int, std::string> p1{42, "hello"};
std::vector<int> v1{1, 2, 3};

// C++17: 可以省略模板参数
std::pair p2{42, "hello"};        // 推导为std::pair<int, const char*>
std::vector v2{1, 2, 3};          // 推导为std::vector<int>

// 自定义类也支持
template<typename T>
class MyClass {
public:
    MyClass(T value) : data(value) {}
private:
    T data;
};

MyClass obj{42};  // 推导为MyClass<int>
```

#### 5. **内联变量**
```cpp
// 头文件中定义全局变量（C++17之前会有多重定义问题）
inline int global_counter = 0;

class MyClass {
public:
    static inline int static_member = 100;  // 可以在类内初始化静态成员
};
```

#### 6. **嵌套命名空间**
```cpp
// C++14及之前
namespace A {
    namespace B {
        namespace C {
            void func() {}
        }
    }
}

// C++17: 简化语法
namespace A::B::C {
    void func() {}
}
```

### 📚 标准库新特性

#### 1. **std::optional**
```cpp
#include <optional>

std::optional<int> divide(int a, int b) {
    if (b == 0) return std::nullopt;  // 表示无值
    return a / b;
}

auto result = divide(10, 3);
if (result) {
    std::cout << "Result: " << *result << std::endl;
} else {
    std::cout << "Division by zero!" << std::endl;
}

// 便捷方法
int value = result.value_or(-1);  // 如果有值返回值，否则返回-1
```

#### 2. **std::variant**
```cpp
#include <variant>

std::variant<int, std::string, double> var;

var = 42;                    // 存储int
var = "hello";               // 存储string
var = 3.14;                  // 存储double

// 访问值
std::visit([](const auto& value) {
    std::cout << "Value: " << value << std::endl;
}, var);

// 检查当前类型
if (std::holds_alternative<std::string>(var)) {
    std::cout << "Contains string: " << std::get<std::string>(var) << std::endl;
}
```

#### 3. **std::any**
```cpp
#include <any>

std::any data;
data = 42;
data = std::string("hello");
data = 3.14;

// 检查类型并获取值
if (data.type() == typeid(double)) {
    double value = std::any_cast<double>(data);
    std::cout << "Double value: " << value << std::endl;
}
```

#### 4. **std::string_view**
```cpp
#include <string_view>

// 轻量级字符串视图，不拥有数据
void process_string(std::string_view sv) {
    std::cout << "Processing: " << sv << std::endl;
}

std::string str = "Hello World";
const char* cstr = "Hello C-string";

process_string(str);      // 从std::string构造
process_string(cstr);     // 从C字符串构造
process_string("literal"); // 从字符串字面量构造

// 子字符串操作（无拷贝）
std::string_view sub = str.substr(0, 5);  // "Hello"
```

#### 5. **文件系统库**
```cpp
#include <filesystem>
namespace fs = std::filesystem;

// 遍历目录
for (const auto& entry : fs::directory_iterator("./")) {
    std::cout << entry.path() << std::endl;
}

// 文件信息
fs::path file_path = "example.txt";
if (fs::exists(file_path)) {
    std::cout << "File size: " << fs::file_size(file_path) << " bytes" << std::endl;
}

// 创建目录
fs::create_directories("path/to/new/directory");
```

#### 6. **并行算法**
```cpp
#include <algorithm>
#include <execution>

std::vector<int> vec(1000000);
std::iota(vec.begin(), vec.end(), 1);

// 并行排序
std::sort(std::execution::par, vec.begin(), vec.end());

// 并行查找
auto count = std::count_if(std::execution::par_unseq, vec.begin(), vec.end(),
                          [](int x) { return x % 2 == 0; });
```

---

## C++20 (2020)
> **革命性更新** - 引入了概念、模块、协程等重大特性

### 🚀 核心语言特性

#### 1. **概念 (Concepts)**
```cpp
#include <concepts>

// 定义概念
template<typename T>
concept Integral = std::is_integral_v<T>;

template<typename T>
concept Addable = requires(T a, T b) {
    a + b;  // 要求类型T支持+操作
};

// 使用概念约束模板
template<Integral T>
T multiply(T a, T b) {
    return a * b;
}

// 或者使用requires子句
template<typename T>
requires Addable<T>
T add(T a, T b) {
    return a + b;
}

// 简化语法
void process(Integral auto value) {
    std::cout << "Processing integer: " << value << std::endl;
}
```

#### 2. **模块 (Modules)**
```cpp
// math_module.cppm (模块接口文件)
export module math_utils;

export int add(int a, int b) {
    return a + b;
}

export namespace math {
    double multiply(double a, double b) {
        return a * b;
    }
}

// main.cpp (使用模块)
import math_utils;

int main() {
    int result = add(5, 3);
    double product = math::multiply(2.5, 4.0);
    return 0;
}
```

#### 3. **协程 (Coroutines)**
```cpp
#include <coroutine>

// 简单的生成器
template<typename T>
struct Generator {
    struct promise_type {
        T current_value;
        auto get_return_object() { return Generator{*this}; }
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        auto yield_value(T value) {
            current_value = value;
            return std::suspend_always{};
        }
        void unhandled_exception() {}
    };
    
    std::coroutine_handle<promise_type> coro;
    
    Generator(promise_type& promise) 
        : coro(std::coroutine_handle<promise_type>::from_promise(promise)) {}
    
    ~Generator() { if (coro) coro.destroy(); }
    
    bool next() {
        coro.resume();
        return !coro.done();
    }
    
    T value() { return coro.promise().current_value; }
};

// 使用协程
Generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;
        auto temp = a;
        a = b;
        b = temp + b;
    }
}
```

#### 4. **三向比较操作符 (<==>)**
```cpp
#include <compare>

class Point {
    int x, y;
public:
    Point(int x, int y) : x(x), y(y) {}
    
    // 自动生成所有比较操作符
    auto operator<=>(const Point& other) const = default;
    
    // 或者自定义比较逻辑
    std::strong_ordering operator<=>(const Point& other) const {
        if (auto cmp = x <=> other.x; cmp != 0) return cmp;
        return y <=> other.y;
    }
};

Point p1{1, 2};
Point p2{3, 4};

bool equal = (p1 == p2);      // false
bool less = (p1 < p2);        // true
auto cmp = (p1 <=> p2);       // std::strong_ordering::less
```

#### 5. **范围 (Ranges)**
```cpp
#include <ranges>
#include <algorithm>

std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// 管道式操作
auto even_squares = numbers 
    | std::views::filter([](int n) { return n % 2 == 0; })
    | std::views::transform([](int n) { return n * n; });

for (int value : even_squares) {
    std::cout << value << " ";  // 输出: 4 16 36 64 100
}

// 范围算法
std::ranges::sort(numbers);
auto it = std::ranges::find(numbers, 5);
```

#### 6. **指定初始化器 (Designated initializers)**
```cpp
struct Point3D {
    int x, y, z;
};

Point3D p1{.x = 10, .y = 20, .z = 30};
Point3D p2{.x = 5, .z = 15};  // y将被默认初始化为0
```

#### 7. **consteval 和 constinit**
```cpp
// consteval: 必须在编译期计算
consteval int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

constexpr int f1 = factorial(5);  // OK，编译期计算
// int n = 5; int f2 = factorial(n);  // 错误，运行期不能调用

// constinit: 必须在编译期初始化
constinit int global_var = factorial(10);  // OK
```

### 📚 标准库新特性

#### 1. **std::span**
```cpp
#include <span>

void process_data(std::span<int> data) {
    for (int& value : data) {
        value *= 2;
    }
}

std::vector<int> vec{1, 2, 3, 4, 5};
std::array<int, 3> arr{10, 20, 30};
int c_array[] = {100, 200, 300};

process_data(vec);                    // 从vector构造span
process_data(std::span{arr});         // 从array构造span
process_data(c_array);                // 从C数组构造span
```

#### 2. **std::format**
```cpp
#include <format>

std::string name = "Alice";
int age = 30;
double height = 5.6;

// 类似Python的格式化字符串
std::string formatted = std::format("Name: {}, Age: {}, Height: {:.1f}", 
                                   name, age, height);
// 输出: "Name: Alice, Age: 30, Height: 5.6"

// 位置参数
std::string msg = std::format("{1} is {0} years old", age, name);
// 输出: "Alice is 30 years old"
```

#### 3. **std::jthread**
```cpp
#include <thread>

// 支持自动停止的线程
std::jthread t([](std::stop_token token) {
    while (!token.stop_requested()) {
        // 执行工作
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
});

// 自动请求停止并等待结束
// t的析构函数会自动调用request_stop()和join()
```

#### 4. **std::semaphore (信号量)**
```cpp
#include <semaphore>

// 计数信号量 - 控制对有限资源的访问
std::counting_semaphore<10> resource_pool(3);  // 最大计数10，初始值3

// 二进制信号量 - 类似mutex但更轻量
std::binary_semaphore signal(0);  // 初始值0（不可用）

void worker(int id) {
    resource_pool.acquire();  // 获取资源（P操作）
    std::cout << "线程 " << id << " 获取资源，开始工作\n";
    
    // 执行工作
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    resource_pool.release();  // 释放资源（V操作）
    std::cout << "线程 " << id << " 释放资源\n";
}

// 非阻塞操作
if (resource_pool.try_acquire()) {
    // 成功获取资源
    resource_pool.release();
}

// 带超时的获取
using namespace std::chrono_literals;
if (resource_pool.try_acquire_for(100ms)) {
    // 在100ms内获取到资源
    resource_pool.release();
}

// 应用场景：连接池、线程池、资源限制
class ConnectionPool {
    std::counting_semaphore<10> available_connections_;
    std::queue<Connection> connections_;
    std::mutex mutex_;
    
public:
    ConnectionPool(size_t size) : available_connections_(size) {
        for (size_t i = 0; i < size; ++i) {
            connections_.emplace();
        }
    }
    
    Connection get_connection() {
        available_connections_.acquire();
        std::lock_guard lock(mutex_);
        auto conn = std::move(connections_.front());
        connections_.pop();
        return conn;
    }
    
    void return_connection(Connection conn) {
        {
            std::lock_guard lock(mutex_);
            connections_.push(std::move(conn));
        }
        available_connections_.release();
    }
};
```

#### 5. **std::barrier 和 std::latch**
```cpp
#include <barrier>
#include <latch>

// 屏障 - 等待所有线程到达某点
std::barrier sync_point(3);  // 3个线程的同步点

void worker() {
    // 第一阶段工作
    sync_point.arrive_and_wait();  // 等待所有线程完成第一阶段
    
    // 第二阶段工作
}

// 门闩 - 一次性同步原语
std::latch done(3);  // 等待3个事件

void task() {
    // 执行任务
    done.count_down();  // 通知任务完成
}

// 等待所有任务完成
done.wait();
```

#### 5. **改进的容器**
```cpp
// std::map::contains
std::map<std::string, int> map{{"key1", 1}, {"key2", 2}};
if (map.contains("key1")) {  // C++20新增
    // 比map.find(key) != map.end()更简洁
}

// std::vector::erase_if
std::vector<int> vec{1, 2, 3, 4, 5};
std::erase_if(vec, [](int x) { return x % 2 == 0; });  // 删除偶数
```

---

## 特性对比表

| 特性类别 | C++11 | C++14 | C++17 | C++20 |
|---------|-------|-------|-------|-------|
| **类型推导** | `auto`, `decltype` | 返回类型推导 | CTAD | 概念约束 |
| **函数式编程** | lambda表达式 | 泛型lambda | 折叠表达式 | 范围和视图 |
| **模板元编程** | 可变参数模板 | 变量模板 | `if constexpr` | 概念 |
| **内存管理** | 智能指针 | `make_unique` | - | - |
| **并发** | `<thread>`, `<mutex>` | - | 并行算法 | `jthread`, 协程 |
| **错误处理** | `noexcept` | - | `optional` | - |
| **字符串** | 原始字符串 | 用户定义字面量 | `string_view` | `format` |
| **容器** | `unordered_*` | - | - | `span` |

### 🎯 学习建议

1. **优先掌握 C++11** - 现代C++的基础
2. **C++14 作为补充** - 相对简单，主要是语法糖
3. **重点学习 C++17** - 实用特性多，大幅提升开发效率
4. **逐步了解 C++20** - 概念和模块较复杂，但功能强大

### 📚 实践项目建议

- **C++11项目**: 使用智能指针、lambda、auto重构旧代码
- **C++17项目**: 使用结构化绑定、optional、string_view
- **C++20项目**: 尝试使用概念约束模板，体验范围库

每个版本都在让C++变得更安全、更高效、更现代化！

---

## 多进程与多线程并发编程指南

### 🧵 **多线程并发手段**

#### 1. **基本同步原语**

##### **std::mutex (互斥锁)**
```cpp
#include <mutex>

std::mutex mtx;
int shared_data = 0;

void thread_function() {
    std::lock_guard<std::mutex> lock(mtx);  // RAII自动解锁
    shared_data++;  // 临界区
}

// 不同类型的mutex
std::recursive_mutex rmtx;      // 可重入锁
std::timed_mutex tmtx;          // 支持超时的锁
std::shared_mutex smtx;         // 读写锁 (C++17)
```

##### **std::condition_variable (条件变量)**
```cpp
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void producer() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
    }
    cv.notify_one();  // 通知一个等待的线程
}

void consumer() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return ready; });  // 等待条件满足
    // 执行消费操作
}
```

##### **std::atomic (原子操作)**
```cpp
#include <atomic>

std::atomic<int> counter{0};
std::atomic<bool> flag{false};

// 无锁操作
counter++;                          // 原子递增
counter.fetch_add(5);               // 原子加法
bool expected = false;
if (flag.compare_exchange_weak(expected, true)) {
    // 成功将false改为true
}

// 内存序控制
counter.store(100, std::memory_order_release);
int value = counter.load(std::memory_order_acquire);
```

#### 2. **高级同步原语 (C++20)**

##### **std::semaphore (信号量)**
```cpp
#include <semaphore>

std::counting_semaphore<5> resource_sem(3);  // 最多3个资源
std::binary_semaphore notification_sem(0);   // 二进制信号量

void worker() {
    resource_sem.acquire();  // 获取资源
    // 执行工作
    resource_sem.release();  // 释放资源
}
```

##### **std::barrier (屏障)**
```cpp
#include <barrier>

std::barrier sync_point(4);  // 4个线程的同步点

void phase_worker(int phase) {
    // 执行当前阶段工作
    std::cout << "Phase " << phase << " work done\n";
    
    sync_point.arrive_and_wait();  // 等待其他线程完成
    
    // 所有线程都到达后继续
    std::cout << "All threads completed phase " << phase << "\n";
}
```

##### **std::latch (门闩)**
```cpp
#include <latch>

std::latch startup_latch(3);  // 等待3个线程启动

void worker() {
    // 初始化工作
    startup_latch.count_down();  // 通知已启动
    
    startup_latch.wait();        // 等待所有线程启动完成
    // 开始主要工作
}
```

#### 3. **线程管理**

##### **std::thread**
```cpp
#include <thread>

void task(int id) {
    std::cout << "Task " << id << " running\n";
}

// 创建和管理线程
std::thread t1(task, 1);
std::thread t2([]() { std::cout << "Lambda thread\n"; });

t1.join();      // 等待线程结束
t2.detach();    // 分离线程

// 线程池概念
class ThreadPool {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
    
public:
    template<class F>
    void enqueue(F&& f) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }
};
```

##### **std::jthread (C++20)**
```cpp
#include <thread>

std::jthread worker([](std::stop_token token) {
    while (!token.stop_requested()) {
        // 执行工作
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
});

// jthread析构时自动stop和join
```

#### 4. **无锁数据结构**

##### **Lock-free队列示例**
```cpp
#include <atomic>

template<typename T>
class LockFreeQueue {
    struct Node {
        std::atomic<T*> data{nullptr};
        std::atomic<Node*> next{nullptr};
    };
    
    std::atomic<Node*> head{new Node};
    std::atomic<Node*> tail{head.load()};
    
public:
    void enqueue(T item) {
        Node* new_node = new Node;
        T* data = new T(std::move(item));
        new_node->data.store(data);
        
        Node* prev_tail = tail.exchange(new_node);
        prev_tail->next.store(new_node);
    }
    
    bool dequeue(T& result) {
        Node* head_node = head.load();
        Node* next = head_node->next.load();
        
        if (next == nullptr) return false;
        
        T* data = next->data.load();
        if (data == nullptr) return false;
        
        result = *data;
        head.store(next);
        delete data;
        delete head_node;
        return true;
    }
};
```

### 🔄 **多进程并发手段**

#### 1. **进程间通信 (IPC)**

##### **共享内存 (POSIX)**
```cpp
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// 现代POSIX共享内存 - 推荐方式
const char* shm_name = "/my_shared_memory";
const size_t shm_size = 4096;

// 创建/打开共享内存对象
int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
if (shm_fd == -1) {
    perror("shm_open failed");
    return;
}

// 设置共享内存大小
if (ftruncate(shm_fd, shm_size) == -1) {
    perror("ftruncate failed");
    close(shm_fd);
    return;
}

// 映射到进程地址空间
void* shared_mem = mmap(nullptr, shm_size, 
                       PROT_READ | PROT_WRITE, 
                       MAP_SHARED, shm_fd, 0);
if (shared_mem == MAP_FAILED) {
    perror("mmap failed");
    close(shm_fd);
    return;
}

// 使用共享内存
strcpy((char*)shared_mem, "Hello from modern process");

// 可选：同步到存储
msync(shared_mem, shm_size, MS_SYNC);

// 清理资源
munmap(shared_mem, shm_size);
close(shm_fd);
shm_unlink(shm_name);  // 删除共享内存对象

// C++现代化封装
class PosixSharedMemory {
private:
    std::string name_;
    size_t size_;
    int fd_;
    void* ptr_;
    
public:
    PosixSharedMemory(const std::string& name, size_t size, bool create = true) 
        : name_(name), size_(size), fd_(-1), ptr_(nullptr) {
        
        int flags = create ? (O_CREAT | O_RDWR) : O_RDWR;
        fd_ = shm_open(name_.c_str(), flags, 0666);
        if (fd_ == -1) {
            throw std::runtime_error("shm_open failed: " + std::string(strerror(errno)));
        }
        
        if (create && ftruncate(fd_, size_) == -1) {
            close(fd_);
            shm_unlink(name_.c_str());
            throw std::runtime_error("ftruncate failed: " + std::string(strerror(errno)));
        }
        
        ptr_ = mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (ptr_ == MAP_FAILED) {
            close(fd_);
            if (create) shm_unlink(name_.c_str());
            throw std::runtime_error("mmap failed: " + std::string(strerror(errno)));
        }
    }
    
    ~PosixSharedMemory() {
        if (ptr_ != nullptr) munmap(ptr_, size_);
        if (fd_ != -1) close(fd_);
    }
    
    template<typename T>
    T* get() { return static_cast<T*>(ptr_); }
    
    void* data() { return ptr_; }
    size_t size() const { return size_; }
    
    void sync() { msync(ptr_, size_, MS_SYNC); }
    
    static void unlink(const std::string& name) {
        shm_unlink(name.c_str());
    }
};

// 使用示例
struct SharedData {
    std::atomic<int> counter{0};
    char message[256];
    std::atomic<bool> ready{false};
};

// 生产者进程
void producer() {
    PosixSharedMemory shm("/producer_consumer", sizeof(SharedData));
    auto* data = shm.get<SharedData>();
    
    strcpy(data->message, "Hello from producer");
    data->counter.store(42);
    data->ready.store(true);
    
    std::cout << "Producer: 数据已写入共享内存\n";
}

// 消费者进程  
void consumer() {
    PosixSharedMemory shm("/producer_consumer", sizeof(SharedData), false);
    auto* data = shm.get<SharedData>();
    
    // 等待数据准备就绪
    while (!data->ready.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "Consumer: 收到消息: " << data->message 
              << ", 计数器: " << data->counter.load() << std::endl;
}
```

##### **消息队列 (POSIX)**
```cpp
#include <mqueue.h>
#include <fcntl.h>

// 现代POSIX消息队列 - 推荐方式
const char* queue_name = "/my_message_queue";

// 设置队列属性
struct mq_attr attr;
attr.mq_flags = 0;
attr.mq_maxmsg = 10;      // 最大消息数
attr.mq_msgsize = 256;    // 最大消息大小
attr.mq_curmsgs = 0;

// 创建/打开消息队列
mqd_t mq = mq_open(queue_name, O_CREAT | O_RDWR, 0666, &attr);
if (mq == (mqd_t)-1) {
    perror("mq_open failed");
    return;
}

// 发送消息
const char* message = "Hello POSIX";
if (mq_send(mq, message, strlen(message), 0) == -1) {
    perror("mq_send failed");
}

// 接收消息 (非阻塞)
char buffer[256];
unsigned int priority;
ssize_t bytes_read = mq_receive(mq, buffer, sizeof(buffer), &priority);
if (bytes_read >= 0) {
    buffer[bytes_read] = '\0';
    std::cout << "收到消息: " << buffer << std::endl;
}

// 带超时的接收
struct timespec timeout;
clock_gettime(CLOCK_REALTIME, &timeout);
timeout.tv_sec += 5;  // 5秒超时
ssize_t result = mq_timedreceive(mq, buffer, sizeof(buffer), &priority, &timeout);

// 清理
mq_close(mq);
mq_unlink(queue_name);

// C++现代化封装
class PosixMessageQueue {
private:
    std::string name_;
    mqd_t mq_;
    size_t max_msg_size_;
    
public:
    PosixMessageQueue(const std::string& name, size_t max_msgs = 10, 
                     size_t max_msg_size = 256, bool create = true) 
        : name_(name), mq_((mqd_t)-1), max_msg_size_(max_msg_size) {
        
        struct mq_attr attr;
        attr.mq_flags = 0;
        attr.mq_maxmsg = max_msgs;
        attr.mq_msgsize = max_msg_size;
        attr.mq_curmsgs = 0;
        
        int flags = create ? (O_CREAT | O_RDWR) : O_RDWR;
        mq_ = mq_open(name_.c_str(), flags, 0666, create ? &attr : nullptr);
        if (mq_ == (mqd_t)-1) {
            throw std::runtime_error("mq_open failed: " + std::string(strerror(errno)));
        }
    }
    
    ~PosixMessageQueue() {
        if (mq_ != (mqd_t)-1) {
            mq_close(mq_);
        }
    }
    
    bool send(const std::string& message, unsigned int priority = 0) {
        return mq_send(mq_, message.c_str(), message.length(), priority) == 0;
    }
    
    std::optional<std::string> receive(std::chrono::milliseconds timeout = std::chrono::milliseconds::zero()) {
        std::vector<char> buffer(max_msg_size_);
        unsigned int priority;
        ssize_t bytes_read;
        
        if (timeout == std::chrono::milliseconds::zero()) {
            bytes_read = mq_receive(mq_, buffer.data(), buffer.size(), &priority);
        } else {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += timeout.count() / 1000;
            ts.tv_nsec += (timeout.count() % 1000) * 1000000;
            
            bytes_read = mq_timedreceive(mq_, buffer.data(), buffer.size(), &priority, &ts);
        }
        
        if (bytes_read >= 0) {
            return std::string(buffer.data(), bytes_read);
        }
        return std::nullopt;
    }
    
    static void unlink(const std::string& name) {
        mq_unlink(name.c_str());
    }
};

// 使用示例
void message_producer() {
    PosixMessageQueue mq("/app_messages");
    
    for (int i = 0; i < 5; ++i) {
        std::string msg = "消息 " + std::to_string(i);
        if (mq.send(msg)) {
            std::cout << "发送: " << msg << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void message_consumer() {
    PosixMessageQueue mq("/app_messages", 10, 256, false);
    
    while (true) {
        auto msg = mq.receive(std::chrono::seconds(5));
        if (msg) {
            std::cout << "接收: " << *msg << std::endl;
        } else {
            std::cout << "超时，退出\n";
            break;
        }
    }
}
```

##### **管道 (Pipe)**
```cpp
#include <unistd.h>
#include <sys/wait.h>

int pipefd[2];
pipe(pipefd);  // 创建管道

if (fork() == 0) {
    // 子进程
    close(pipefd[1]);  // 关闭写端
    char buffer[100];
    read(pipefd[0], buffer, sizeof(buffer));
    std::cout << "Child received: " << buffer << std::endl;
    close(pipefd[0]);
} else {
    // 父进程
    close(pipefd[0]);  // 关闭读端
    write(pipefd[1], "Hello child", 11);
    close(pipefd[1]);
    wait(nullptr);  // 等待子进程结束
}
```

##### **命名管道 (FIFO)**
```cpp
#include <sys/stat.h>
#include <fcntl.h>

// 创建命名管道
mkfifo("/tmp/myfifo", 0666);

// 写进程
int fd = open("/tmp/myfifo", O_WRONLY);
write(fd, "Hello", 5);
close(fd);

// 读进程
int fd = open("/tmp/myfifo", O_RDONLY);
char buffer[100];
read(fd, buffer, sizeof(buffer));
close(fd);
```

#### 2. **进程同步**

##### **POSIX信号量**
```cpp
#include <semaphore.h>
#include <fcntl.h>

// 创建命名信号量
sem_t* sem = sem_open("/my_semaphore", O_CREAT, 0666, 1);

// P操作 (获取)
sem_wait(sem);

// 临界区
std::cout << "Process in critical section\n";

// V操作 (释放)
sem_post(sem);

// 关闭和删除
sem_close(sem);
sem_unlink("/my_semaphore");
```

##### **文件锁**
```cpp
#include <fcntl.h>
#include <unistd.h>

int fd = open("lockfile", O_CREAT | O_RDWR, 0666);

struct flock fl;
fl.l_type = F_WRLCK;    // 写锁
fl.l_whence = SEEK_SET;
fl.l_start = 0;
fl.l_len = 0;           // 锁定整个文件

// 获取锁
if (fcntl(fd, F_SETLKW, &fl) == -1) {
    perror("fcntl");
}

// 临界区操作

// 释放锁
fl.l_type = F_UNLCK;
fcntl(fd, F_SETLK, &fl);
close(fd);
```

#### 3. **现代Linux特有IPC机制**

##### **eventfd - 事件通知**
```cpp
#include <sys/eventfd.h>
#include <unistd.h>

// 创建事件文件描述符
int create_event_notifier() {
    int efd = eventfd(0, EFD_CLOEXEC);  // 初始值为0
    if (efd == -1) {
        perror("eventfd failed");
        return -1;
    }
    return efd;
}

// 信号通知
void signal_event(int efd, uint64_t value = 1) {
    ssize_t s = write(efd, &value, sizeof(value));
    if (s != sizeof(value)) {
        perror("write to eventfd failed");
    }
}

// 等待事件
uint64_t wait_for_event(int efd) {
    uint64_t value;
    ssize_t s = read(efd, &value, sizeof(value));
    if (s == sizeof(value)) {
        return value;
    }
    return 0;
}

// C++封装的事件通知器
class EventNotifier {
private:
    int efd_;
    
public:
    EventNotifier() : efd_(eventfd(0, EFD_CLOEXEC)) {
        if (efd_ == -1) {
            throw std::runtime_error("eventfd creation failed");
        }
    }
    
    ~EventNotifier() {
        if (efd_ != -1) close(efd_);
    }
    
    void notify(uint64_t count = 1) {
        if (write(efd_, &count, sizeof(count)) != sizeof(count)) {
            throw std::runtime_error("Failed to notify event");
        }
    }
    
    uint64_t wait() {
        uint64_t count;
        if (read(efd_, &count, sizeof(count)) == sizeof(count)) {
            return count;
        }
        throw std::runtime_error("Failed to wait for event");
    }
    
    int fd() const { return efd_; }
};

// 使用示例：生产者-消费者
void producer_with_eventfd() {
    EventNotifier notifier;
    
    std::thread producer([&notifier]() {
        for (int i = 0; i < 10; ++i) {
            // 生产数据
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            notifier.notify();  // 通知有新数据
        }
    });
    
    std::thread consumer([&notifier]() {
        for (int i = 0; i < 10; ++i) {
            uint64_t events = notifier.wait();  // 等待通知
            std::cout << "消费者收到 " << events << " 个事件\n";
        }
    });
    
    producer.join();
    consumer.join();
}
```

##### **signalfd - 信号处理**
```cpp
#include <sys/signalfd.h>
#include <signal.h>

// 现代信号处理方式
class SignalHandler {
private:
    int sfd_;
    sigset_t mask_;
    
public:
    SignalHandler(std::initializer_list<int> signals) {
        sigemptyset(&mask_);
        for (int sig : signals) {
            sigaddset(&mask_, sig);
        }
        
        // 阻塞信号，通过signalfd处理
        if (pthread_sigmask(SIG_BLOCK, &mask_, nullptr) == -1) {
            throw std::runtime_error("pthread_sigmask failed");
        }
        
        sfd_ = signalfd(-1, &mask_, SFD_CLOEXEC);
        if (sfd_ == -1) {
            throw std::runtime_error("signalfd failed");
        }
    }
    
    ~SignalHandler() {
        if (sfd_ != -1) close(sfd_);
    }
    
    int wait_for_signal() {
        struct signalfd_siginfo si;
        ssize_t s = read(sfd_, &si, sizeof(si));
        if (s == sizeof(si)) {
            return si.ssi_signo;
        }
        return -1;
    }
    
    int fd() const { return sfd_; }
};

// 使用示例
void signal_handling_example() {
    SignalHandler handler{SIGINT, SIGTERM, SIGUSR1};
    
    std::cout << "等待信号...\n";
    while (true) {
        int sig = handler.wait_for_signal();
        switch (sig) {
            case SIGINT:
                std::cout << "收到 SIGINT，优雅退出\n";
                return;
            case SIGTERM:
                std::cout << "收到 SIGTERM，立即退出\n";
                return;
            case SIGUSR1:
                std::cout << "收到 SIGUSR1，用户自定义信号\n";
                break;
            default:
                std::cout << "未知信号: " << sig << std::endl;
        }
    }
}
```

##### **timerfd - 定时器**
```cpp
#include <sys/timerfd.h>
#include <time.h>

class Timer {
private:
    int tfd_;
    
public:
    Timer() : tfd_(timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC)) {
        if (tfd_ == -1) {
            throw std::runtime_error("timerfd_create failed");
        }
    }
    
    ~Timer() {
        if (tfd_ != -1) close(tfd_);
    }
    
    void start(std::chrono::milliseconds interval, std::chrono::milliseconds initial_delay = std::chrono::milliseconds::zero()) {
        struct itimerspec timer_spec;
        
        // 初始延迟
        auto delay = initial_delay.count() > 0 ? initial_delay : interval;
        timer_spec.it_value.tv_sec = delay.count() / 1000;
        timer_spec.it_value.tv_nsec = (delay.count() % 1000) * 1000000;
        
        // 重复间隔
        timer_spec.it_interval.tv_sec = interval.count() / 1000;
        timer_spec.it_interval.tv_nsec = (interval.count() % 1000) * 1000000;
        
        if (timerfd_settime(tfd_, 0, &timer_spec, nullptr) == -1) {
            throw std::runtime_error("timerfd_settime failed");
        }
    }
    
    void stop() {
        struct itimerspec timer_spec = {};  // 全部设为0停止定时器
        timerfd_settime(tfd_, 0, &timer_spec, nullptr);
    }
    
    uint64_t wait() {
        uint64_t expirations;
        if (read(tfd_, &expirations, sizeof(expirations)) == sizeof(expirations)) {
            return expirations;
        }
        return 0;
    }
    
    int fd() const { return tfd_; }
};

// 使用示例：定时任务
void timer_example() {
    Timer timer;
    timer.start(std::chrono::seconds(1));  // 每秒触发一次
    
    for (int i = 0; i < 5; ++i) {
        uint64_t expirations = timer.wait();
        std::cout << "定时器触发，过期次数: " << expirations << std::endl;
    }
    
    timer.stop();
}
```

##### **epoll - 高性能I/O多路复用**
```cpp
#include <sys/epoll.h>
#include <vector>

class EPollReactor {
private:
    int epfd_;
    std::vector<struct epoll_event> events_;
    
public:
    EPollReactor(size_t max_events = 1024) 
        : epfd_(epoll_create1(EPOLL_CLOEXEC)), events_(max_events) {
        if (epfd_ == -1) {
            throw std::runtime_error("epoll_create1 failed");
        }
    }
    
    ~EPollReactor() {
        if (epfd_ != -1) close(epfd_);
    }
    
    void add_fd(int fd, uint32_t events = EPOLLIN, void* data = nullptr) {
        struct epoll_event ev;
        ev.events = events;
        ev.data.ptr = data;
        ev.data.fd = fd;
        
        if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
            throw std::runtime_error("epoll_ctl ADD failed");
        }
    }
    
    void remove_fd(int fd) {
        epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
    }
    
    int wait(int timeout_ms = -1) {
        return epoll_wait(epfd_, events_.data(), events_.size(), timeout_ms);
    }
    
    const struct epoll_event& get_event(int index) const {
        return events_[index];
    }
};

// 使用示例：事件驱动服务器
void event_driven_example() {
    EPollReactor reactor;
    EventNotifier notifier;
    Timer timer;
    SignalHandler signal_handler{SIGINT, SIGTERM};
    
    // 添加各种事件源
    reactor.add_fd(notifier.fd(), EPOLLIN, (void*)1);
    reactor.add_fd(timer.fd(), EPOLLIN, (void*)2);
    reactor.add_fd(signal_handler.fd(), EPOLLIN, (void*)3);
    
    timer.start(std::chrono::seconds(2));  // 2秒定时器
    
    std::cout << "事件循环启动...\n";
    while (true) {
        int nfds = reactor.wait();
        
        for (int i = 0; i < nfds; ++i) {
            const auto& event = reactor.get_event(i);
            void* type = event.data.ptr;
            
            if (type == (void*)1) {  // EventNotifier
                uint64_t count = notifier.wait();
                std::cout << "收到事件通知: " << count << std::endl;
            } else if (type == (void*)2) {  // Timer
                uint64_t expirations = timer.wait();
                std::cout << "定时器触发: " << expirations << std::endl;
                notifier.notify();  // 触发事件通知
            } else if (type == (void*)3) {  // SignalHandler
                int sig = signal_handler.wait_for_signal();
                std::cout << "收到信号: " << sig << "，退出\n";
                return;
            }
        }
    }
}
```

##### **内存映射 (mmap)**
```cpp
#include <sys/mman.h>
#include <fcntl.h>

// 创建文件
int fd = open("shared_file", O_CREAT | O_RDWR, 0666);
ftruncate(fd, 4096);

// 映射到内存
void* mapped = mmap(nullptr, 4096, PROT_READ | PROT_WRITE, 
                   MAP_SHARED, fd, 0);

// 使用映射内存
strcpy((char*)mapped, "Shared data");

// 同步到文件
msync(mapped, 4096, MS_SYNC);

// 解除映射
munmap(mapped, 4096);
close(fd);
```

### 📊 **并发手段对比表**

| 并发场景 | 多线程方案 | 多进程方案 | 优缺点对比 |
|---------|-----------|-----------|------------|
| **数据共享** | 共享内存空间 | 共享内存/内存映射 | 线程：简单但需同步<br/>进程：复杂但隔离性好 |
| **通信** | 直接访问变量 | IPC机制 | 线程：高效<br/>进程：开销大但安全 |
| **同步** | mutex/atomic | 信号量/文件锁 | 线程：轻量级<br/>进程：重量级但跨进程 |
| **错误隔离** | 共享地址空间 | 独立地址空间 | 线程：一个崩溃全崩溃<br/>进程：独立错误处理 |
| **创建开销** | 轻量级 | 重量级 | 线程：微秒级<br/>进程：毫秒级 |
| **调试难度** | 高（竞态条件） | 中等 | 线程：时序敏感<br/>进程：相对独立 |

### 🎯 **选择指南**

#### **选择多线程的场景：**
- 需要频繁共享数据
- 对性能要求高
- 计算密集型任务
- 需要细粒度并行控制

#### **选择多进程的场景：**
- 需要强错误隔离
- 处理不可信数据
- 需要利用多机器资源
- 长期运行的服务

#### **混合模式：**
- 进程间分工，进程内多线程
- 例如：Web服务器多进程处理请求，每个进程内多线程处理连接

### 🛡️ **最佳实践**

#### **多线程最佳实践：**
```cpp
// 1. 使用RAII管理锁
{
    std::lock_guard<std::mutex> lock(mtx);
    // 自动解锁
}

// 2. 避免死锁 - 锁排序
void transfer(Account& from, Account& to, int amount) {
    std::lock(from.mutex, to.mutex);  // 同时获取两个锁
    std::lock_guard<std::mutex> lock1(from.mutex, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(to.mutex, std::adopt_lock);
    // 执行转账
}

// 3. 使用线程安全的容器
std::atomic<int> counter{0};  // 替代mutex保护的int

// 4. 减少锁的粒度
class ThreadSafeMap {
    mutable std::shared_mutex mutex_;
    std::map<Key, Value> map_;
public:
    Value get(const Key& key) const {
        std::shared_lock lock(mutex_);  // 读锁
        return map_.at(key);
    }
    
    void set(const Key& key, const Value& value) {
        std::unique_lock lock(mutex_);  // 写锁
        map_[key] = value;
    }
};
```

#### **多进程最佳实践：**
```cpp
// 1. 优雅处理子进程
signal(SIGCHLD, SIG_IGN);  // 避免僵尸进程

// 2. 使用命名资源进行同步
sem_t* sem = sem_open("/unique_name", O_CREAT | O_EXCL, 0666, 1);

// 3. 错误处理和清理
void cleanup() {
    sem_unlink("/unique_name");
    shmctl(shmid, IPC_RMID, nullptr);
}
atexit(cleanup);

// 4. 设置合适的权限
shmget(key, size, IPC_CREAT | 0600);  // 只有创建者可访问
```

这个全面的指南涵盖了C++中多线程和多进程编程的各种并发手段，帮助你选择最适合的方案！
