# C++ STL 容器和适配器成员方法汇总

## 目录
- [序列容器](#序列容器)
  - [vector](#vector)
  - [deque](#deque)
  - [list](#list)
  - [forward_list](#forward_list)
  - [array](#array)
- [关联容器](#关联容器)
  - [set/multiset](#setmultiset)
  - [map/multimap](#mapmultimap)
- [无序关联容器](#无序关联容器)
  - [unordered_set/unordered_multiset](#unordered_setunordered_multiset)
  - [unordered_map/unordered_multimap](#unordered_mapunordered_multimap)
- [容器适配器](#容器适配器)
  - [stack](#stack)
  - [queue](#queue)
  - [priority_queue](#priority_queue)
- [字符串](#字符串)
  - [string](#string)

---

## 序列容器

### vector
动态数组，连续内存存储

#### 构造和析构
```cpp
vector()                          // 默认构造
vector(size_t n)                  // 构造n个默认元素
vector(size_t n, const T& val)    // 构造n个val
vector(const vector& other)       // 拷贝构造
vector(vector&& other)            // 移动构造
vector(initializer_list<T> il)    // 初始化列表构造
vector(InputIt first, InputIt last) // 迭代器范围构造
```

#### 容量操作
```cpp
size_t size() const               // 元素个数
size_t capacity() const           // 容量大小
bool empty() const                // 是否为空
size_t max_size() const           // 最大可能大小
void reserve(size_t n)            // 预留容量
void shrink_to_fit()              // 释放多余容量
void resize(size_t n)             // 改变大小
void resize(size_t n, const T& val) // 改变大小并填充值
```

#### 元素访问
```cpp
T& operator[](size_t index)       // 下标访问（不检查边界）
const T& operator[](size_t index) const
T& at(size_t index)               // 下标访问（检查边界）
const T& at(size_t index) const
T& front()                        // 第一个元素
const T& front() const
T& back()                         // 最后一个元素
const T& back() const
T* data()                         // 底层数组指针
const T* data() const
```

#### 迭代器
```cpp
iterator begin()                  // 开始迭代器
const_iterator begin() const
const_iterator cbegin() const
iterator end()                    // 结束迭代器
const_iterator end() const
const_iterator cend() const
reverse_iterator rbegin()         // 反向开始迭代器
const_reverse_iterator rbegin() const
const_reverse_iterator crbegin() const
reverse_iterator rend()           // 反向结束迭代器
const_reverse_iterator rend() const
const_reverse_iterator crend() const
```

#### 修改操作
```cpp
void push_back(const T& val)      // 尾部添加元素
void push_back(T&& val)
template<class... Args>
void emplace_back(Args&&... args) // 尾部原地构造
void pop_back()                   // 删除尾部元素
iterator insert(const_iterator pos, const T& val) // 插入元素
iterator insert(const_iterator pos, T&& val)
iterator insert(const_iterator pos, size_t n, const T& val)
template<class InputIt>
iterator insert(const_iterator pos, InputIt first, InputIt last)
iterator insert(const_iterator pos, initializer_list<T> il)
template<class... Args>
iterator emplace(const_iterator pos, Args&&... args) // 原地构造插入
iterator erase(const_iterator pos) // 删除元素
iterator erase(const_iterator first, const_iterator last)
void clear()                      // 清空容器
void swap(vector& other)          // 交换容器
vector& operator=(const vector& other) // 赋值操作符
vector& operator=(vector&& other)
vector& operator=(initializer_list<T> il)
void assign(size_t n, const T& val) // 赋值操作
template<class InputIt>
void assign(InputIt first, InputIt last)
void assign(initializer_list<T> il)
```

---

### deque
双端队列，分段连续内存

#### 特有操作（相比vector额外增加）
```cpp
void push_front(const T& val)     // 头部添加元素
void push_front(T&& val)
template<class... Args>
void emplace_front(Args&&... args) // 头部原地构造
void pop_front()                  // 删除头部元素
```

*其他方法与vector基本相同*

---

### list
双向链表

#### 构造和基本操作
*与vector类似的构造函数、容量操作、迭代器*

#### 元素访问（注意：无随机访问）
```cpp
T& front()                        // 第一个元素
const T& front() const
T& back()                         // 最后一个元素  
const T& back() const
// 注意：list没有operator[]和at()方法
```

#### 修改操作
```cpp
void push_front(const T& val)     // 头部添加
void push_front(T&& val)
void push_back(const T& val)      // 尾部添加
void push_back(T&& val)
template<class... Args>
void emplace_front(Args&&... args) // 头部原地构造
template<class... Args>
void emplace_back(Args&&... args)  // 尾部原地构造
void pop_front()                  // 删除头部
void pop_back()                   // 删除尾部
iterator insert(const_iterator pos, const T& val) // 插入
// ... 其他insert重载
template<class... Args>
iterator emplace(const_iterator pos, Args&&... args)
iterator erase(const_iterator pos) // 删除
iterator erase(const_iterator first, const_iterator last)
```

#### list特有操作
```cpp
void splice(const_iterator pos, list& other) // 拼接链表
void splice(const_iterator pos, list& other, const_iterator it)
void splice(const_iterator pos, list& other, 
            const_iterator first, const_iterator last)
void remove(const T& val)         // 删除特定值
template<class Predicate>
void remove_if(Predicate pred)    // 按条件删除
void unique()                     // 删除连续重复元素
template<class BinaryPredicate>
void unique(BinaryPredicate pred)
void merge(list& other)           // 合并有序链表
template<class Compare>
void merge(list& other, Compare comp)
void sort()                       // 排序
template<class Compare>
void sort(Compare comp)
void reverse()                    // 反转
```

---

### forward_list
单向链表

#### 特点
- 只支持前向迭代器
- 没有size()方法
- 插入删除操作使用_after版本

#### 特有操作
```cpp
T& front()                        // 访问第一个元素
void push_front(const T& val)     // 头部添加
template<class... Args>
void emplace_front(Args&&... args)
void pop_front()                  // 删除头部

iterator before_begin()           // 首元素前的位置
const_iterator before_begin() const
const_iterator cbefore_begin() const

iterator insert_after(const_iterator pos, const T& val) // 在pos后插入
// ... 其他insert_after重载
template<class... Args>
iterator emplace_after(const_iterator pos, Args&&... args)
iterator erase_after(const_iterator pos) // 删除pos后的元素
iterator erase_after(const_iterator first, const_iterator last)

void splice_after(const_iterator pos, forward_list& other)
// ... 其他splice_after重载
void remove(const T& val)
template<class Predicate>
void remove_if(Predicate pred)
void unique()
void merge(forward_list& other)
void sort()
void reverse()
```

---

### array
固定大小数组

#### 特点
- 编译时确定大小
- 支持随机访问
- 栈上分配

#### 主要操作
```cpp
// 元素访问
T& operator[](size_t index)
T& at(size_t index)
T& front()
T& back()
T* data()

// 容量
constexpr size_t size() const     // 数组大小
constexpr size_t max_size() const
constexpr bool empty() const

// 迭代器
iterator begin()
iterator end()
reverse_iterator rbegin()
reverse_iterator rend()

// 操作
void fill(const T& val)           // 填充所有元素
void swap(array& other)           // 交换
```

---

## 关联容器

### set/multiset
有序集合（基于红黑树）

#### 构造
```cpp
set()                            // 默认构造
explicit set(const Compare& comp) // 指定比较函数
set(const set& other)            // 拷贝构造
set(set&& other)                 // 移动构造
set(initializer_list<T> il)      // 初始化列表
template<class InputIt>
set(InputIt first, InputIt last) // 迭代器范围
```

#### 容量
```cpp
bool empty() const
size_t size() const
size_t max_size() const
```

#### 迭代器
```cpp
iterator begin()
const_iterator begin() const
const_iterator cbegin() const
iterator end()
const_iterator end() const  
const_iterator cend() const
reverse_iterator rbegin()
const_reverse_iterator crbegin() const
reverse_iterator rend()
const_reverse_iterator crend() const
```

#### 修改操作
```cpp
pair<iterator, bool> insert(const T& val) // 插入元素
pair<iterator, bool> insert(T&& val)
iterator insert(const_iterator hint, const T& val) // 带提示插入
iterator insert(const_iterator hint, T&& val)
template<class InputIt>
void insert(InputIt first, InputIt last)
void insert(initializer_list<T> il)
template<class... Args>
pair<iterator, bool> emplace(Args&&... args) // 原地构造
template<class... Args>
iterator emplace_hint(const_iterator hint, Args&&... args)
iterator erase(const_iterator pos)  // 删除
iterator erase(const_iterator first, const_iterator last)
size_t erase(const T& key)          // 按键删除
void clear()
void swap(set& other)
```

#### 查找操作
```cpp
size_t count(const T& key) const    // 计数（set返回0或1）
iterator find(const T& key)         // 查找
const_iterator find(const T& key) const
iterator lower_bound(const T& key)  // 下界
const_iterator lower_bound(const T& key) const
iterator upper_bound(const T& key)  // 上界
const_iterator upper_bound(const T& key) const
pair<iterator, iterator> equal_range(const T& key) // 相等范围
pair<const_iterator, const_iterator> equal_range(const T& key) const
```

#### 观察器
```cpp
key_compare key_comp() const        // 键比较函数
value_compare value_comp() const    // 值比较函数
```

---

### map/multimap
有序键值对容器

#### 构造
*与set类似*

#### 元素访问（仅map）
```cpp
T& operator[](const Key& key)      // 下标访问
T& operator[](Key&& key)
T& at(const Key& key)              // 检查边界的访问
const T& at(const Key& key) const
```

#### 修改操作
```cpp
pair<iterator, bool> insert(const value_type& val) // value_type是pair<Key,T>
template<class P>
pair<iterator, bool> insert(P&& val)
iterator insert(const_iterator hint, const value_type& val)
template<class InputIt>
void insert(InputIt first, InputIt last)
void insert(initializer_list<value_type> il)
template<class... Args>
pair<iterator, bool> emplace(Args&&... args)
template<class... Args>
iterator emplace_hint(const_iterator hint, Args&&... args)
// 删除操作与set相同
```

#### 查找操作
*与set相同，但key类型是Key*

---

## 无序关联容器

### unordered_set/unordered_multiset
基于哈希表的集合

#### 主要接口
*大部分与set相同，但增加了哈希表特有操作*

#### 哈希表操作
```cpp
size_t bucket_count() const        // 桶数量
size_t max_bucket_count() const    // 最大桶数量
size_t bucket_size(size_t n) const // 指定桶的大小
size_t bucket(const Key& key) const // key所在的桶
float load_factor() const          // 负载因子
float max_load_factor() const      // 最大负载因子
void max_load_factor(float z)      // 设置最大负载因子
void rehash(size_t n)              // 重新哈希
void reserve(size_t n)             // 预留空间
```

#### 哈希相关函数
```cpp
hasher hash_function() const       // 哈希函数对象
key_equal key_eq() const           // 键相等判断函数
```

---

### unordered_map/unordered_multimap
基于哈希表的键值对容器

*接口与unordered_set类似，但键值对操作与map相同*

---

## 容器适配器

### stack
栈适配器（LIFO）

#### 构造
```cpp
stack()                           // 默认构造
explicit stack(const Container& cont) // 基于容器构造
explicit stack(Container&& cont)
stack(const stack& other)         // 拷贝构造
stack(stack&& other)              // 移动构造
```

#### 元素访问
```cpp
T& top()                          // 访问栈顶
const T& top() const
```

#### 容量
```cpp
bool empty() const                // 是否为空
size_t size() const               // 元素个数
```

#### 修改操作
```cpp
void push(const T& val)           // 入栈
void push(T&& val)
template<class... Args>
void emplace(Args&&... args)      // 原地构造入栈
void pop()                        // 出栈（不返回元素）
void swap(stack& other)           // 交换
```

---

### queue
队列适配器（FIFO）

#### 构造
*与stack类似*

#### 元素访问
```cpp
T& front()                        // 访问队首
const T& front() const
T& back()                         // 访问队尾
const T& back() const
```

#### 容量
```cpp
bool empty() const
size_t size() const
```

#### 修改操作
```cpp
void push(const T& val)           // 入队
void push(T&& val)
template<class... Args>
void emplace(Args&&... args)      // 原地构造入队
void pop()                        // 出队（不返回元素）
void swap(queue& other)
```

---

### priority_queue
优先队列（最大堆）

#### 构造
```cpp
priority_queue()                  // 默认构造
explicit priority_queue(const Compare& comp) // 指定比较函数
priority_queue(const Compare& comp, const Container& cont)
priority_queue(const Compare& comp, Container&& cont)
template<class InputIt>
priority_queue(InputIt first, InputIt last) // 范围构造
priority_queue(const priority_queue& other)
priority_queue(priority_queue&& other)
```

#### 元素访问
```cpp
const T& top() const              // 访问堆顶（最大元素）
```

#### 容量
```cpp
bool empty() const
size_t size() const
```

#### 修改操作
```cpp
void push(const T& val)           // 插入元素
void push(T&& val)
template<class... Args>
void emplace(Args&&... args)      // 原地构造插入
void pop()                        // 删除堆顶元素
void swap(priority_queue& other)
```

---

## 字符串

### string
字符串类

#### 构造
```cpp
string()                          // 空字符串
string(const string& str)         // 拷贝构造
string(const string& str, size_t pos) // 从位置pos开始的子串
string(const string& str, size_t pos, size_t len) // 子串
string(const char* s)             // C字符串构造
string(const char* s, size_t n)   // 前n个字符
string(size_t n, char c)          // n个字符c
template<class InputIt>
string(InputIt first, InputIt last) // 迭代器范围
string(initializer_list<char> il) // 初始化列表
```

#### 容量操作
```cpp
size_t size() const               // 字符数
size_t length() const             // 字符数（同size）
size_t capacity() const           // 容量
bool empty() const                // 是否为空
size_t max_size() const           // 最大可能大小
void reserve(size_t n)            // 预留容量
void shrink_to_fit()              // 收缩容量
void resize(size_t n)             // 改变大小
void resize(size_t n, char c)     // 改变大小并填充字符
```

#### 元素访问
```cpp
char& operator[](size_t pos)      // 下标访问
const char& operator[](size_t pos) const
char& at(size_t pos)              // 检查边界访问
const char& at(size_t pos) const
char& front()                     // 第一个字符
const char& front() const
char& back()                      // 最后一个字符
const char& back() const
const char* data() const          // C字符串指针
const char* c_str() const         // C字符串指针（保证null结尾）
```

#### 修改操作
```cpp
string& operator+=(const string& str) // 追加字符串
string& operator+=(const char* s)
string& operator+=(char c)
string& operator+=(initializer_list<char> il)
string& append(const string& str)  // 追加操作
string& append(const string& str, size_t subpos, size_t sublen)
string& append(const char* s)
string& append(const char* s, size_t n)
string& append(size_t n, char c)
template<class InputIt>
string& append(InputIt first, InputIt last)
string& append(initializer_list<char> il)

void push_back(char c)            // 尾部添加字符
void pop_back()                   // 删除尾部字符

string& assign(const string& str) // 赋值操作
// ... 其他assign重载

string& insert(size_t pos, const string& str) // 插入操作
// ... 其他insert重载

string& erase(size_t pos = 0, size_t len = npos) // 删除操作
iterator erase(const_iterator p)
iterator erase(const_iterator first, const_iterator last)

string& replace(size_t pos, size_t len, const string& str) // 替换操作
// ... 其他replace重载

void swap(string& str)            // 交换
void clear()                      // 清空
```

#### 字符串操作
```cpp
const char* c_str() const         // 获取C字符串
const char* data() const          // 获取数据指针
size_t copy(char* s, size_t len, size_t pos = 0) const // 复制到C数组

size_t find(const string& str, size_t pos = 0) const // 查找
size_t find(const char* s, size_t pos = 0) const
size_t find(const char* s, size_t pos, size_t n) const
size_t find(char c, size_t pos = 0) const

size_t rfind(const string& str, size_t pos = npos) const // 反向查找
// ... 其他rfind重载

size_t find_first_of(const string& str, size_t pos = 0) const // 查找任一字符
// ... 其他find_first_of重载

size_t find_last_of(const string& str, size_t pos = npos) const // 反向查找任一字符
// ... 其他find_last_of重载

size_t find_first_not_of(const string& str, size_t pos = 0) const // 查找不匹配字符
// ... 其他find_first_not_of重载

size_t find_last_not_of(const string& str, size_t pos = npos) const // 反向查找不匹配字符
// ... 其他find_last_not_of重载

string substr(size_t pos = 0, size_t len = npos) const // 子字符串

int compare(const string& str) const // 比较字符串
int compare(size_t pos, size_t len, const string& str) const
int compare(size_t pos, size_t len, const string& str, 
           size_t subpos, size_t sublen) const
int compare(const char* s) const
int compare(size_t pos, size_t len, const char* s) const
int compare(size_t pos, size_t len, const char* s, size_t n) const
```

---

## 通用操作符重载

### 比较操作符
大多数容器都支持以下比较操作符：
```cpp
bool operator==(const Container& lhs, const Container& rhs)
bool operator!=(const Container& lhs, const Container& rhs)
bool operator<(const Container& lhs, const Container& rhs)
bool operator<=(const Container& lhs, const Container& rhs)
bool operator>(const Container& lhs, const Container& rhs)
bool operator>=(const Container& lhs, const Container& rhs)
```

### 非成员函数
```cpp
void swap(Container& x, Container& y)  // 交换两个容器
```

---

## 常用常量

### string特有常量
```cpp
static const size_t npos = -1;       // 表示"直到字符串结尾"或"未找到"
```

---

## 迭代器类型总结

- **RandomAccessIterator**: vector, deque, array, string
- **BidirectionalIterator**: list, set, map, multiset, multimap
- **ForwardIterator**: forward_list, unordered_set, unordered_map, unordered_multiset, unordered_multimap
- **无迭代器**: stack, queue, priority_queue（适配器）

---

*注：此文档基于C++17标准，某些方法可能在更早或更新的标准中有所不同*
