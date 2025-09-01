# Linux 高性能并发编程指南

## 📋 目录
- [多线程并发技术](#多线程并发技术)
- [多进程并发技术](#多进程并发技术)
- [Linux特有高性能机制](#linux特有高性能机制)
- [无锁编程技术](#无锁编程技术)
- [内存管理与优化](#内存管理与优化)
- [性能调优技巧](#性能调优技巧)
- [实战案例与基准测试](#实战案例与基准测试)

---

## 🧵 多线程并发技术

### **1. 标准线程库 (C++11/14/17/20)**

#### **基础线程操作**
```cpp
#include <thread>
#include <iostream>
#include <vector>
#include <chrono>

// 高性能线程创建
class HighPerformanceThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;

public:
    HighPerformanceThreadPool(size_t threads) : stop(false) {
        // 根据CPU核心数创建线程
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                // 设置CPU亲和性
                cpu_set_t cpuset;
                CPU_ZERO(&cpuset);
                CPU_SET(sched_getcpu(), &cpuset);
                pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
                
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty()) return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
        }
    }
};
```

#### **C++20 高性能特性**
```cpp
#include <coroutine>
#include <semaphore>
#include <barrier>
#include <latch>
#include <jthread>

// C++20 协程 - 零拷贝异步I/O
struct AsyncTask {
    struct promise_type {
        AsyncTask get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

AsyncTask async_network_io() {
    // 使用 io_uring 进行异步I/O
    co_await schedule_io_operation();
    co_return;
}

// C++20 同步原语 - 高性能屏障
class HighPerformanceBarrier {
    std::barrier<> sync_point;
public:
    HighPerformanceBarrier(std::ptrdiff_t count) : sync_point(count) {}
    
    void wait() {
        sync_point.arrive_and_wait();
    }
};

// C++20 信号量 - 资源池管理
class ResourcePool {
    std::counting_semaphore<1000> available_resources{1000};
    
public:
    void acquire_resource() {
        available_resources.acquire();  // 阻塞直到资源可用
    }
    
    void release_resource() {
        available_resources.release();
    }
};
```

### **2. POSIX 线程 (pthread) - 底层控制**

#### **高性能线程属性设置**
```cpp
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>

class OptimizedPThread {
public:
    static pthread_t create_high_performance_thread(void* (*start_routine)(void*), void* arg) {
        pthread_t thread;
        pthread_attr_t attr;
        
        // 初始化线程属性
        pthread_attr_init(&attr);
        
        // 设置为分离状态 - 避免内存泄漏
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        
        // 设置栈大小 - 减少内存开销
        size_t stack_size = 1024 * 1024;  // 1MB栈
        pthread_attr_setstacksize(&attr, stack_size);
        
        // 设置调度策略 - 实时优先级
        struct sched_param param;
        param.sched_priority = 50;
        pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
        pthread_attr_setschedparam(&attr, &param);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        
        pthread_create(&thread, &attr, start_routine, arg);
        pthread_attr_destroy(&attr);
        
        return thread;
    }
    
    // CPU亲和性设置
    static void set_cpu_affinity(pthread_t thread, int cpu_id) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_id, &cpuset);
        pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    }
};
```

#### **高性能互斥锁**
```cpp
#include <pthread.h>

class FastMutex {
private:
    pthread_mutex_t mutex;
    
public:
    FastMutex() {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        
        // 设置为适应性互斥锁 - 短时间自旋，长时间阻塞
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ADAPTIVE_NP);
        
        // 设置进程间共享 (如果需要)
        // pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        
        pthread_mutex_init(&mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }
    
    ~FastMutex() {
        pthread_mutex_destroy(&mutex);
    }
    
    void lock() { pthread_mutex_lock(&mutex); }
    void unlock() { pthread_mutex_unlock(&mutex); }
    bool try_lock() { return pthread_mutex_trylock(&mutex) == 0; }
};

// 读写锁 - 读多写少场景优化
class HighPerformanceRWLock {
private:
    pthread_rwlock_t rwlock;
    
public:
    HighPerformanceRWLock() {
        pthread_rwlockattr_t attr;
        pthread_rwlockattr_init(&attr);
        
        // 优先写者策略
        pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
        
        pthread_rwlock_init(&rwlock, &attr);
        pthread_rwlockattr_destroy(&attr);
    }
    
    void read_lock() { pthread_rwlock_rdlock(&rwlock); }
    void write_lock() { pthread_rwlock_wrlock(&rwlock); }
    void unlock() { pthread_rwlock_unlock(&rwlock); }
};
```

### **3. 线程本地存储 (TLS) 优化**

```cpp
#include <thread>

// C++11 thread_local - 编译器优化
class ThreadLocalCache {
    static thread_local char buffer[4096];  // 每线程4KB缓冲区
    static thread_local size_t buffer_pos;
    
public:
    static void* fast_allocate(size_t size) {
        if (buffer_pos + size <= sizeof(buffer)) {
            void* ptr = buffer + buffer_pos;
            buffer_pos += size;
            return ptr;
        }
        return malloc(size);  // 回退到堆分配
    }
};

// POSIX TLS - 手动控制
class PosixTLS {
private:
    static pthread_key_t key;
    static pthread_once_t key_once;
    
    static void make_key() {
        pthread_key_create(&key, destructor);
    }
    
    static void destructor(void* ptr) {
        free(ptr);
    }
    
public:
    static void* get_data() {
        pthread_once(&key_once, make_key);
        return pthread_getspecific(key);
    }
    
    static void set_data(void* data) {
        pthread_once(&key_once, make_key);
        pthread_setspecific(key, data);
    }
};
```

---

## 🔄 多进程并发技术

### **1. 进程创建与管理**

#### **高性能进程池**
```cpp
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

class ProcessPool {
private:
    std::vector<pid_t> workers;
    int pipe_fd[2];
    
public:
    ProcessPool(size_t num_processes) {
        pipe(pipe_fd);
        
        for (size_t i = 0; i < num_processes; ++i) {
            pid_t pid = fork();
            if (pid == 0) {
                // 子进程
                worker_process();
                exit(0);
            } else if (pid > 0) {
                workers.push_back(pid);
            }
        }
    }
    
    void worker_process() {
        // 设置进程优先级
        setpriority(PRIO_PROCESS, 0, -10);
        
        // 绑定CPU核心
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(getpid() % sysconf(_SC_NPROCESSORS_ONLN), &cpuset);
        sched_setaffinity(0, sizeof(cpuset), &cpuset);
        
        // 工作循环
        while (true) {
            // 处理任务
            process_task();
        }
    }
    
    ~ProcessPool() {
        for (pid_t pid : workers) {
            kill(pid, SIGTERM);
            waitpid(pid, nullptr, 0);
        }
    }
};
```

### **2. 高性能进程间通信 (IPC)**

#### **POSIX 共享内存 - 零拷贝通信**
```cpp
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

template<typename T>
class HighPerformanceSharedMemory {
private:
    int shm_fd;
    T* data;
    sem_t* read_sem;
    sem_t* write_sem;
    size_t size;
    
public:
    HighPerformanceSharedMemory(const char* name, size_t count) : size(sizeof(T) * count) {
        // 创建共享内存
        shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
        ftruncate(shm_fd, size);
        
        // 映射到进程地址空间
        data = static_cast<T*>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
        
        // 使用大页面优化 (需要root权限或配置)
        madvise(data, size, MADV_HUGEPAGE);
        
        // 创建信号量
        read_sem = sem_open((std::string(name) + "_read").c_str(), O_CREAT, 0666, 0);
        write_sem = sem_open((std::string(name) + "_write").c_str(), O_CREAT, 0666, count);
    }
    
    // 生产者写入
    void write(const T& item, size_t index) {
        sem_wait(write_sem);        // 等待可写空间
        
        data[index] = item;         // 零拷贝写入
        __sync_synchronize();       // 内存屏障
        
        sem_post(read_sem);         // 通知可读
    }
    
    // 消费者读取
    T read(size_t index) {
        sem_wait(read_sem);         // 等待可读数据
        
        T result = data[index];     // 零拷贝读取
        __sync_synchronize();       // 内存屏障
        
        sem_post(write_sem);        // 通知可写
        return result;
    }
    
    ~HighPerformanceSharedMemory() {
        munmap(data, size);
        close(shm_fd);
        sem_close(read_sem);
        sem_close(write_sem);
    }
};
```

#### **POSIX 消息队列 - 高吞吐量**
```cpp
#include <mqueue.h>

class HighThroughputMessageQueue {
private:
    mqd_t mq;
    struct mq_attr attr;
    
public:
    HighThroughputMessageQueue(const char* name, size_t max_msgs = 1000, size_t msg_size = 8192) {
        attr.mq_flags = 0;
        attr.mq_maxmsg = max_msgs;
        attr.mq_msgsize = msg_size;
        attr.mq_curmsgs = 0;
        
        mq = mq_open(name, O_CREAT | O_RDWR | O_NONBLOCK, 0666, &attr);
        if (mq == -1) {
            throw std::runtime_error("Failed to create message queue");
        }
    }
    
    // 非阻塞发送 - 高性能
    bool try_send(const void* msg_ptr, size_t msg_len, unsigned int msg_prio = 0) {
        return mq_send(mq, static_cast<const char*>(msg_ptr), msg_len, msg_prio) == 0;
    }
    
    // 批量发送优化
    size_t batch_send(const std::vector<std::pair<const void*, size_t>>& messages) {
        size_t sent = 0;
        for (const auto& [msg_ptr, msg_len] : messages) {
            if (try_send(msg_ptr, msg_len)) {
                ++sent;
            } else {
                break;  // 队列满，停止发送
            }
        }
        return sent;
    }
    
    // 非阻塞接收
    ssize_t try_receive(void* msg_ptr, size_t msg_len, unsigned int* msg_prio = nullptr) {
        return mq_receive(mq, static_cast<char*>(msg_ptr), msg_len, msg_prio);
    }
    
    ~HighThroughputMessageQueue() {
        mq_close(mq);
    }
};
```

#### **管道优化 - 高带宽传输**
```cpp
#include <sys/socket.h>
#include <fcntl.h>

class HighBandwidthPipe {
private:
    int pipe_fd[2];
    
public:
    HighBandwidthPipe() {
        // 使用 socketpair 代替 pipe - 双向通信
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, pipe_fd) == -1) {
            throw std::runtime_error("Failed to create socketpair");
        }
        
        // 设置为非阻塞
        fcntl(pipe_fd[0], F_SETFL, O_NONBLOCK);
        fcntl(pipe_fd[1], F_SETFL, O_NONBLOCK);
        
        // 增大缓冲区
        int buffer_size = 1024 * 1024;  // 1MB
        setsockopt(pipe_fd[0], SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));
        setsockopt(pipe_fd[1], SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));
    }
    
    // 零拷贝发送 (使用 splice)
    ssize_t splice_send(int src_fd, size_t len) {
        return splice(src_fd, nullptr, pipe_fd[1], nullptr, len, SPLICE_F_MOVE);
    }
    
    // 批量写入
    ssize_t writev(const struct iovec* iov, int iovcnt) {
        return ::writev(pipe_fd[1], iov, iovcnt);
    }
    
    // 批量读取
    ssize_t readv(const struct iovec* iov, int iovcnt) {
        return ::readv(pipe_fd[0], iov, iovcnt);
    }
    
    int get_read_fd() const { return pipe_fd[0]; }
    int get_write_fd() const { return pipe_fd[1]; }
    
    ~HighBandwidthPipe() {
        close(pipe_fd[0]);
        close(pipe_fd[1]);
    }
};
```

---

## ⚡ Linux特有高性能机制

### **1. epoll - 高并发I/O多路复用**

```cpp
#include <sys/epoll.h>
#include <sys/eventfd.h>

class HighPerformanceEpoll {
private:
    int epoll_fd;
    struct epoll_event* events;
    int max_events;
    
public:
    HighPerformanceEpoll(int max_events = 1000) : max_events(max_events) {
        // 创建 epoll 实例
        epoll_fd = epoll_create1(EPOLL_CLOEXEC);
        events = new struct epoll_event[max_events];
    }
    
    // 添加文件描述符 - 边缘触发模式
    void add_fd(int fd, uint32_t events_mask = EPOLLIN | EPOLLET) {
        struct epoll_event ev;
        ev.events = events_mask;
        ev.data.fd = fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
    }
    
    // 高性能事件循环
    void event_loop() {
        while (true) {
            int nfds = epoll_wait(epoll_fd, events, max_events, -1);
            
            for (int i = 0; i < nfds; ++i) {
                if (events[i].events & EPOLLIN) {
                    handle_read(events[i].data.fd);
                }
                if (events[i].events & EPOLLOUT) {
                    handle_write(events[i].data.fd);
                }
                if (events[i].events & EPOLLERR) {
                    handle_error(events[i].data.fd);
                }
            }
        }
    }
    
private:
    void handle_read(int fd) { /* 处理读事件 */ }
    void handle_write(int fd) { /* 处理写事件 */ }
    void handle_error(int fd) { /* 处理错误事件 */ }
    
public:
    ~HighPerformanceEpoll() {
        delete[] events;
        close(epoll_fd);
    }
};
```

### **2. io_uring - 异步I/O最新技术**

```cpp
#include <liburing.h>

class IOUringAsyncIO {
private:
    struct io_uring ring;
    constexpr static int QUEUE_DEPTH = 256;
    
public:
    IOUringAsyncIO() {
        // 初始化 io_uring
        if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
            throw std::runtime_error("Failed to initialize io_uring");
        }
    }
    
    // 异步读取
    void async_read(int fd, void* buf, size_t len, off_t offset, void* user_data) {
        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
        io_uring_prep_read(sqe, fd, buf, len, offset);
        io_uring_sqe_set_data(sqe, user_data);
        io_uring_submit(&ring);
    }
    
    // 异步写入
    void async_write(int fd, const void* buf, size_t len, off_t offset, void* user_data) {
        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
        io_uring_prep_write(sqe, fd, buf, len, offset);
        io_uring_sqe_set_data(sqe, user_data);
        io_uring_submit(&ring);
    }
    
    // 批量提交 - 减少系统调用
    void submit_batch() {
        io_uring_submit(&ring);
    }
    
    // 等待完成
    int wait_completion(void** user_data) {
        struct io_uring_cqe* cqe;
        int ret = io_uring_wait_cqe(&ring, &cqe);
        if (ret == 0) {
            *user_data = io_uring_cqe_get_data(cqe);
            int result = cqe->res;
            io_uring_cqe_seen(&ring, cqe);
            return result;
        }
        return ret;
    }
    
    ~IOUringAsyncIO() {
        io_uring_queue_exit(&ring);
    }
};
```

### **3. eventfd - 轻量级事件通知**

```cpp
#include <sys/eventfd.h>

class EventNotifier {
private:
    int event_fd;
    
public:
    EventNotifier() {
        event_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        if (event_fd == -1) {
            throw std::runtime_error("Failed to create eventfd");
        }
    }
    
    // 发送通知
    void notify(uint64_t value = 1) {
        write(event_fd, &value, sizeof(value));
    }
    
    // 接收通知
    uint64_t wait() {
        uint64_t value;
        read(event_fd, &value, sizeof(value));
        return value;
    }
    
    int get_fd() const { return event_fd; }
    
    ~EventNotifier() {
        close(event_fd);
    }
};

// 跨进程事件通知系统
class CrossProcessEventSystem {
private:
    std::shared_ptr<HighPerformanceSharedMemory<EventNotifier>> shared_notifiers;
    
public:
    CrossProcessEventSystem(const char* name, size_t num_events) {
        shared_notifiers = std::make_shared<HighPerformanceSharedMemory<EventNotifier>>(name, num_events);
    }
    
    void notify_process(int process_id, uint64_t event_data) {
        shared_notifiers->write(EventNotifier(), process_id);
    }
};
```

### **4. timerfd - 高精度定时器**

```cpp
#include <sys/timerfd.h>

class HighPrecisionTimer {
private:
    int timer_fd;
    
public:
    HighPrecisionTimer() {
        timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
        if (timer_fd == -1) {
            throw std::runtime_error("Failed to create timerfd");
        }
    }
    
    // 设置周期性定时器 - 纳秒级精度
    void set_periodic(long interval_ns) {
        struct itimerspec timer_spec;
        timer_spec.it_interval.tv_sec = interval_ns / 1000000000;
        timer_spec.it_interval.tv_nsec = interval_ns % 1000000000;
        timer_spec.it_value = timer_spec.it_interval;
        
        timerfd_settime(timer_fd, 0, &timer_spec, nullptr);
    }
    
    // 等待定时器触发
    uint64_t wait() {
        uint64_t expirations;
        read(timer_fd, &expirations, sizeof(expirations));
        return expirations;
    }
    
    int get_fd() const { return timer_fd; }
    
    ~HighPrecisionTimer() {
        close(timer_fd);
    }
};
```

---

## 🔒 无锁编程技术

### **1. 原子操作与内存序**

```cpp
#include <atomic>

// 高性能无锁计数器
class LockFreeCounter {
private:
    std::atomic<uint64_t> counter{0};
    
public:
    // 原子递增 - 最快
    uint64_t increment() {
        return counter.fetch_add(1, std::memory_order_relaxed);
    }
    
    // 原子递增 - 获取-修改-存储语义
    uint64_t increment_acq_rel() {
        return counter.fetch_add(1, std::memory_order_acq_rel);
    }
    
    // 原子比较交换
    bool compare_exchange(uint64_t expected, uint64_t desired) {
        return counter.compare_exchange_weak(expected, desired, std::memory_order_acq_rel);
    }
    
    uint64_t load() const {
        return counter.load(std::memory_order_acquire);
    }
};

// 无锁栈 - Michael & Scott 算法
template<typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        std::atomic<Node*> next;
        Node(T data) : data(std::move(data)), next(nullptr) {}
    };
    
    std::atomic<Node*> head{nullptr};
    
public:
    void push(T item) {
        Node* new_node = new Node(std::move(item));
        new_node->next = head.load(std::memory_order_relaxed);
        
        while (!head.compare_exchange_weak(new_node->next, new_node,
                                          std::memory_order_release,
                                          std::memory_order_relaxed)) {
            // CAS 失败，重试
        }
    }
    
    bool pop(T& result) {
        Node* old_head = head.load(std::memory_order_acquire);
        
        while (old_head && !head.compare_exchange_weak(old_head, old_head->next,
                                                      std::memory_order_release,
                                                      std::memory_order_relaxed)) {
            // CAS 失败，重试
        }
        
        if (old_head) {
            result = std::move(old_head->data);
            delete old_head;
            return true;
        }
        return false;
    }
};
```

### **2. 无锁队列 - 高吞吐量**

```cpp
// 单生产者单消费者无锁队列 - 最高性能
template<typename T, size_t SIZE>
class SPSCLockFreeQueue {
private:
    static_assert((SIZE & (SIZE - 1)) == 0, "SIZE must be power of 2");
    
    struct alignas(64) {  // 避免false sharing
        std::atomic<size_t> head{0};
    };
    
    struct alignas(64) {
        std::atomic<size_t> tail{0};
    };
    
    T buffer[SIZE];
    
public:
    // 生产者入队
    bool enqueue(const T& item) {
        size_t current_tail = tail.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) & (SIZE - 1);
        
        if (next_tail == head.load(std::memory_order_acquire)) {
            return false;  // 队列满
        }
        
        buffer[current_tail] = item;
        tail.store(next_tail, std::memory_order_release);
        return true;
    }
    
    // 消费者出队
    bool dequeue(T& item) {
        size_t current_head = head.load(std::memory_order_relaxed);
        
        if (current_head == tail.load(std::memory_order_acquire)) {
            return false;  // 队列空
        }
        
        item = buffer[current_head];
        head.store((current_head + 1) & (SIZE - 1), std::memory_order_release);
        return true;
    }
    
    size_t size() const {
        size_t h = head.load(std::memory_order_relaxed);
        size_t t = tail.load(std::memory_order_relaxed);
        return (t - h) & (SIZE - 1);
    }
};

// 多生产者多消费者无锁队列
template<typename T>
class MPMCLockFreeQueue {
private:
    struct Node {
        std::atomic<T*> data{nullptr};
        std::atomic<Node*> next{nullptr};
    };
    
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
    
public:
    MPMCLockFreeQueue() {
        Node* dummy = new Node;
        head.store(dummy);
        tail.store(dummy);
    }
    
    void enqueue(T item) {
        Node* new_node = new Node;
        T* data = new T(std::move(item));
        new_node->data.store(data);
        
        while (true) {
            Node* last = tail.load();
            Node* next = last->next.load();
            
            if (last == tail.load()) {
                if (next == nullptr) {
                    if (last->next.compare_exchange_weak(next, new_node)) {
                        break;
                    }
                } else {
                    tail.compare_exchange_weak(last, next);
                }
            }
        }
        tail.compare_exchange_weak(tail.load(), new_node);
    }
    
    bool dequeue(T& result) {
        while (true) {
            Node* first = head.load();
            Node* last = tail.load();
            Node* next = first->next.load();
            
            if (first == head.load()) {
                if (first == last) {
                    if (next == nullptr) {
                        return false;  // 队列空
                    }
                    tail.compare_exchange_weak(last, next);
                } else {
                    T* data = next->data.load();
                    if (data == nullptr) {
                        continue;
                    }
                    if (head.compare_exchange_weak(first, next)) {
                        result = *data;
                        delete data;
                        delete first;
                        return true;
                    }
                }
            }
        }
    }
};
```

### **3. 内存回收 - Hazard Pointer**

```cpp
// 危险指针 - 安全内存回收
template<typename T>
class HazardPointer {
private:
    static thread_local T* hazard_ptr;
    static std::atomic<T*> retired_list;
    
public:
    static void protect(T* ptr) {
        hazard_ptr = ptr;
        std::atomic_thread_fence(std::memory_order_seq_cst);
    }
    
    static void clear() {
        hazard_ptr = nullptr;
    }
    
    static void retire(T* ptr) {
        // 将指针加入退休列表
        ptr->next_retired = retired_list.load();
        while (!retired_list.compare_exchange_weak(ptr->next_retired, ptr)) {
            // 重试
        }
        
        // 定期清理
        if (should_cleanup()) {
            cleanup_retired();
        }
    }
    
private:
    static void cleanup_retired() {
        // 扫描所有线程的危险指针
        std::set<T*> hazard_ptrs = collect_hazard_pointers();
        
        // 回收不在危险指针中的对象
        T* current = retired_list.exchange(nullptr);
        while (current) {
            T* next = current->next_retired;
            if (hazard_ptrs.find(current) == hazard_ptrs.end()) {
                delete current;
            } else {
                retire(current);  // 重新加入退休列表
            }
            current = next;
        }
    }
};
```

---

## 💾 内存管理与优化

### **1. 内存池技术**

```cpp
#include <sys/mman.h>

// 高性能内存池
class HighPerformanceMemoryPool {
private:
    struct Block {
        Block* next;
    };
    
    void* memory_start;
    size_t block_size;
    size_t total_size;
    Block* free_list;
    std::atomic<size_t> allocated_count{0};
    
public:
    HighPerformanceMemoryPool(size_t block_size, size_t num_blocks) 
        : block_size(std::max(block_size, sizeof(Block))), 
          total_size(this->block_size * num_blocks) {
        
        // 使用 mmap 分配大页内存
        memory_start = mmap(nullptr, total_size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        
        if (memory_start == MAP_FAILED) {
            throw std::runtime_error("Failed to allocate memory pool");
        }
        
        // 建议使用大页面
        madvise(memory_start, total_size, MADV_HUGEPAGE);
        
        // 初始化空闲链表
        init_free_list(num_blocks);
    }
    
    void* allocate() {
        if (!free_list) {
            return nullptr;  // 内存池耗尽
        }
        
        Block* block = free_list;
        free_list = block->next;
        allocated_count.fetch_add(1, std::memory_order_relaxed);
        
        return block;
    }
    
    void deallocate(void* ptr) {
        if (!ptr) return;
        
        Block* block = static_cast<Block*>(ptr);
        block->next = free_list;
        free_list = block;
        allocated_count.fetch_sub(1, std::memory_order_relaxed);
    }
    
private:
    void init_free_list(size_t num_blocks) {
        char* current = static_cast<char*>(memory_start);
        free_list = reinterpret_cast<Block*>(current);
        
        for (size_t i = 0; i < num_blocks - 1; ++i) {
            Block* block = reinterpret_cast<Block*>(current);
            current += block_size;
            block->next = reinterpret_cast<Block*>(current);
        }
        
        // 最后一个块
        reinterpret_cast<Block*>(current)->next = nullptr;
    }
    
public:
    ~HighPerformanceMemoryPool() {
        munmap(memory_start, total_size);
    }
};
```

### **2. NUMA 感知内存分配**

```cpp
#include <numa.h>
#include <numaif.h>

class NUMAMemoryManager {
private:
    int num_nodes;
    std::vector<void*> node_memory;
    
public:
    NUMAMemoryManager() {
        if (numa_available() == -1) {
            throw std::runtime_error("NUMA not available");
        }
        
        num_nodes = numa_max_node() + 1;
        node_memory.resize(num_nodes, nullptr);
    }
    
    // 在指定NUMA节点分配内存
    void* allocate_on_node(size_t size, int node) {
        void* ptr = numa_alloc_onnode(size, node);
        if (!ptr) {
            throw std::runtime_error("Failed to allocate on NUMA node");
        }
        return ptr;
    }
    
    // 在当前CPU所在NUMA节点分配内存
    void* allocate_local(size_t size) {
        int current_node = numa_node_of_cpu(sched_getcpu());
        return allocate_on_node(size, current_node);
    }
    
    // 绑定内存到NUMA节点
    void bind_memory(void* ptr, size_t size, int node) {
        unsigned long nodemask = 1UL << node;
        mbind(ptr, size, MPOL_BIND, &nodemask, sizeof(nodemask) * 8, 0);
    }
    
    // 获取内存所在NUMA节点
    int get_memory_node(void* ptr) {
        int node;
        get_mempolicy(&node, nullptr, 0, ptr, MPOL_F_NODE | MPOL_F_ADDR);
        return node;
    }
    
    void deallocate(void* ptr, size_t size) {
        numa_free(ptr, size);
    }
};
```

### **3. 缓存友好的数据结构**

```cpp
// 缓存行对齐的数据结构
struct alignas(64) CacheLinePadded {
    std::atomic<uint64_t> value{0};
    // 填充到整个缓存行，避免false sharing
};

// 紧凑的结构体数组 - 提高缓存局部性
template<typename T>
class CacheFriendlyVector {
private:
    static constexpr size_t CACHE_LINE_SIZE = 64;
    static constexpr size_t ELEMENTS_PER_CACHE_LINE = CACHE_LINE_SIZE / sizeof(T);
    
    T* data;
    size_t size_;
    size_t capacity_;
    
public:
    CacheFriendlyVector(size_t initial_capacity) : size_(0) {
        // 分配缓存行对齐的内存
        capacity_ = (initial_capacity + ELEMENTS_PER_CACHE_LINE - 1) & 
                   ~(ELEMENTS_PER_CACHE_LINE - 1);
        
        if (posix_memalign(reinterpret_cast<void**>(&data), CACHE_LINE_SIZE, 
                          capacity_ * sizeof(T)) != 0) {
            throw std::runtime_error("Failed to allocate aligned memory");
        }
        
        // 预取内存到缓存
        for (size_t i = 0; i < capacity_; i += ELEMENTS_PER_CACHE_LINE) {
            __builtin_prefetch(&data[i], 1, 3);  // 写预取，高局部性
        }
    }
    
    // 批量操作 - 充分利用缓存
    template<typename Func>
    void batch_process(Func&& func, size_t batch_size = ELEMENTS_PER_CACHE_LINE) {
        for (size_t i = 0; i < size_; i += batch_size) {
            size_t end = std::min(i + batch_size, size_);
            
            // 预取下一批数据
            if (i + batch_size < size_) {
                __builtin_prefetch(&data[i + batch_size], 0, 3);
            }
            
            // 处理当前批
            for (size_t j = i; j < end; ++j) {
                func(data[j]);
            }
        }
    }
    
    ~CacheFriendlyVector() {
        free(data);
    }
};
```

---

## 🎯 性能调优技巧

### **1. CPU 亲和性与调度优化**

```cpp
#include <sched.h>
#include <pthread.h>

class CPUAffinityManager {
public:
    // 绑定当前线程到指定CPU核心
    static void bind_to_cpu(int cpu_id) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_id, &cpuset);
        
        if (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) != 0) {
            throw std::runtime_error("Failed to set CPU affinity");
        }
    }
    
    // 绑定到多个CPU核心
    static void bind_to_cpus(const std::vector<int>& cpu_ids) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        
        for (int cpu_id : cpu_ids) {
            CPU_SET(cpu_id, &cpuset);
        }
        
        pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
    }
    
    // 设置实时调度策略
    static void set_realtime_priority(int priority = 50) {
        struct sched_param param;
        param.sched_priority = priority;
        
        if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
            // 需要root权限，降级到SCHED_OTHER
            param.sched_priority = 0;
            sched_setscheduler(0, SCHED_OTHER, &param);
            setpriority(PRIO_PROCESS, 0, -20);  // 最高nice值
        }
    }
    
    // 获取CPU拓扑信息
    static std::vector<std::vector<int>> get_cpu_topology() {
        std::vector<std::vector<int>> topology;
        int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
        
        // 按NUMA节点分组CPU
        for (int node = 0; node <= numa_max_node(); ++node) {
            std::vector<int> node_cpus;
            for (int cpu = 0; cpu < num_cpus; ++cpu) {
                if (numa_node_of_cpu(cpu) == node) {
                    node_cpus.push_back(cpu);
                }
            }
            if (!node_cpus.empty()) {
                topology.push_back(node_cpus);
            }
        }
        
        return topology;
    }
};
```

### **2. 内存屏障与缓存控制**

```cpp
#include <emmintrin.h>  // SSE2

class MemoryBarrierOptimization {
public:
    // 编译器屏障 - 防止指令重排
    static void compiler_barrier() {
        asm volatile("" ::: "memory");
    }
    
    // CPU 内存屏障
    static void memory_fence() {
        std::atomic_thread_fence(std::memory_order_seq_cst);
    }
    
    // 轻量级读屏障
    static void read_barrier() {
        std::atomic_thread_fence(std::memory_order_acquire);
    }
    
    // 轻量级写屏障
    static void write_barrier() {
        std::atomic_thread_fence(std::memory_order_release);
    }
    
    // 缓存行刷新
    static void flush_cache_line(const void* addr) {
        _mm_clflush(addr);
    }
    
    // 非临时存储 - 绕过缓存
    static void non_temporal_store(void* addr, __m128i data) {
        _mm_stream_si128(static_cast<__m128i*>(addr), data);
    }
    
    // 预取数据到缓存
    template<int locality>
    static void prefetch(const void* addr) {
        static_assert(locality >= 0 && locality <= 3, "Locality must be 0-3");
        __builtin_prefetch(addr, 0, locality);
    }
    
    // 预取用于写入
    template<int locality>
    static void prefetch_for_write(const void* addr) {
        static_assert(locality >= 0 && locality <= 3, "Locality must be 0-3");
        __builtin_prefetch(addr, 1, locality);
    }
};
```

### **3. 分支预测优化**

```cpp
// 分支预测提示
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

class BranchOptimization {
public:
    // 热路径优化
    template<typename T>
    static T fast_max(T a, T b) {
        if (LIKELY(a > b)) {
            return a;
        } else {
            return b;
        }
    }
    
    // 错误处理路径优化
    static bool validate_input(const void* ptr, size_t size) {
        if (UNLIKELY(!ptr)) {
            handle_null_pointer_error();
            return false;
        }
        
        if (UNLIKELY(size == 0)) {
            handle_zero_size_error();
            return false;
        }
        
        return true;
    }
    
    // 循环展开优化
    static void optimized_copy(const char* src, char* dst, size_t size) {
        size_t i = 0;
        
        // 按8字节对齐复制
        for (; i + 8 <= size; i += 8) {
            *reinterpret_cast<uint64_t*>(dst + i) = 
                *reinterpret_cast<const uint64_t*>(src + i);
        }
        
        // 处理剩余字节
        for (; i < size; ++i) {
            dst[i] = src[i];
        }
    }
    
private:
    static void handle_null_pointer_error() { /* 错误处理 */ }
    static void handle_zero_size_error() { /* 错误处理 */ }
};
```

---

## 📊 实战案例与基准测试

### **1. 高性能服务器架构**

```cpp
// 生产级高性能服务器
class HighPerformanceServer {
private:
    HighPerformanceEpoll epoll;
    std::vector<std::thread> worker_threads;
    SPSCLockFreeQueue<int, 4096> connection_queue;
    HighPerformanceMemoryPool memory_pool;
    
public:
    HighPerformanceServer(int num_workers = std::thread::hardware_concurrency()) 
        : memory_pool(4096, 10000) {  // 4KB块，10000个
        
        // 启动工作线程
        for (int i = 0; i < num_workers; ++i) {
            worker_threads.emplace_back([this, i]() {
                // 绑定CPU
                CPUAffinityManager::bind_to_cpu(i % sysconf(_SC_NPROCESSORS_ONLN));
                
                // 设置实时优先级
                CPUAffinityManager::set_realtime_priority();
                
                // 工作循环
                worker_loop();
            });
        }
    }
    
    void worker_loop() {
        while (true) {
            int client_fd;
            if (connection_queue.dequeue(client_fd)) {
                handle_client(client_fd);
            } else {
                // 没有新连接，让出CPU
                std::this_thread::yield();
            }
        }
    }
    
    void handle_client(int client_fd) {
        // 使用内存池分配缓冲区
        void* buffer = memory_pool.allocate();
        if (!buffer) {
            close(client_fd);
            return;
        }
        
        // 处理请求
        ssize_t bytes_read = read(client_fd, buffer, 4096);
        if (bytes_read > 0) {
            // 处理数据
            process_request(buffer, bytes_read);
            
            // 发送响应
            send_response(client_fd, buffer);
        }
        
        // 归还缓冲区
        memory_pool.deallocate(buffer);
        close(client_fd);
    }
    
private:
    void process_request(void* data, size_t size) { /* 业务逻辑 */ }
    void send_response(int fd, void* data) { /* 发送响应 */ }
};
```

### **2. 性能基准测试工具**

```cpp
#include <chrono>
#include <iomanip>

class PerformanceBenchmark {
private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    
    TimePoint start_time;
    std::string test_name;
    
public:
    PerformanceBenchmark(const std::string& name) : test_name(name) {
        start_time = Clock::now();
    }
    
    ~PerformanceBenchmark() {
        auto end_time = Clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time);
        
        std::cout << std::fixed << std::setprecision(2)
                  << "[" << test_name << "] 耗时: " << duration.count() 
                  << " 微秒" << std::endl;
    }
    
    // 吞吐量测试
    static void throughput_test(const std::string& name, 
                               std::function<void()> operation,
                               size_t iterations) {
        auto start = Clock::now();
        
        for (size_t i = 0; i < iterations; ++i) {
            operation();
        }
        
        auto end = Clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        double ops_per_second = (iterations * 1000000.0) / duration.count();
        
        std::cout << std::fixed << std::setprecision(0)
                  << "[" << name << "] 吞吐量: " << ops_per_second 
                  << " 操作/秒" << std::endl;
    }
    
    // 延迟测试
    static void latency_test(const std::string& name,
                           std::function<void()> operation,
                           size_t iterations) {
        std::vector<uint64_t> latencies;
        latencies.reserve(iterations);
        
        for (size_t i = 0; i < iterations; ++i) {
            auto start = Clock::now();
            operation();
            auto end = Clock::now();
            
            latencies.push_back(
                std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
            );
        }
        
        std::sort(latencies.begin(), latencies.end());
        
        std::cout << "[" << name << "] 延迟统计 (纳秒):\n"
                  << "  最小值: " << latencies[0] << "\n"
                  << "  P50: " << latencies[iterations / 2] << "\n"
                  << "  P90: " << latencies[iterations * 9 / 10] << "\n"
                  << "  P99: " << latencies[iterations * 99 / 100] << "\n"
                  << "  最大值: " << latencies.back() << std::endl;
    }
};

// 使用示例
void run_performance_tests() {
    // 测试无锁队列 vs 互斥锁队列
    SPSCLockFreeQueue<int, 1024> lockfree_queue;
    
    PerformanceBenchmark::throughput_test("无锁队列入队", [&]() {
        lockfree_queue.enqueue(42);
    }, 1000000);
    
    PerformanceBenchmark::throughput_test("无锁队列出队", [&]() {
        int value;
        lockfree_queue.dequeue(value);
    }, 1000000);
    
    // 测试内存分配性能
    HighPerformanceMemoryPool pool(64, 10000);
    
    PerformanceBenchmark::latency_test("内存池分配", [&]() {
        void* ptr = pool.allocate();
        pool.deallocate(ptr);
    }, 100000);
}
```

---

## 🚀 总结与最佳实践

### **性能优化原则**
1. **测量驱动优化** - 先测量，再优化，避免过早优化
2. **热点路径优化** - 专注于占用CPU时间最多的代码路径
3. **缓存友好设计** - 数据结构和算法要考虑缓存局部性
4. **减少系统调用** - 批量操作，减少用户态-内核态切换
5. **NUMA感知编程** - 在多NUMA节点系统上考虑内存访问模式

### **并发设计模式**
1. **生产者-消费者** - 使用无锁队列解耦生产和消费
2. **工作窃取** - 多线程任务分发的高效模式
3. **读写分离** - 使用RCU或读写锁优化读多写少场景
4. **事件驱动** - 使用epoll/io_uring实现高并发I/O
5. **批量处理** - 减少锁竞争和系统调用开销

### **调试与监控**
```bash
# 性能分析工具
perf stat ./your_app                    # CPU性能统计
perf record -g ./your_app              # 性能热点分析
perf mem record ./your_app             # 内存访问分析

# 系统监控
top -H                                 # 查看线程CPU使用
iostat                                 # I/O统计
numastat                              # NUMA内存统计
```

Linux高性能并发编程需要深入理解硬件特性、操作系统机制和编程语言特性。选择合适的并发模型和优化技术，能够显著提升应用性能！
