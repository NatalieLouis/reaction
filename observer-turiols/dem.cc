#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// 线程池实现（用于异步任务）
class ThreadPool {
 public:
  ThreadPool(size_t threads) : stop(false) {
    for (size_t i = 0; i < threads; ++i) {
      workers.emplace_back([this] {
        for (;;) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
            if (this->stop && this->tasks.empty()) return;
            task = std::move(this->tasks.top().second);
            this->tasks.pop();
          }
          task();
        }
      });
    }
  }

  // 提交带优先级的任务（1为最高优先级）
  template <class F>
  void enqueue(int priority, F&& f) {
    auto task = std::make_shared<std::packaged_task<void()>>(std::forward<F>(f));
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
      tasks.emplace(priority, [task]() { (*task)(); });
    }
    condition.notify_one();
  }

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers) worker.join();
  }

 private:
  std::vector<std::thread> workers;
  // 优先队列：按优先级排序任务（小值优先）
  std::priority_queue<std::pair<int, std::function<void()>>> tasks;
  std::mutex queue_mutex;
  std::condition_variable condition;
  std::atomic<bool> stop;
};

// 事件类型枚举（细分DEM状态变化事件）
enum class DEMStateEvent {
  DeadToActive,  // 从死亡到活跃
  ActiveToDead,  // 从活跃到死亡
  ActiveToIdle,  // 从活跃到空闲
  IdleToActive   // 从空闲到活跃
};

// 事件数据结构（携带事件类型和额外信息）
struct DEMEvent {
  DEMStateEvent type;
  std::string dem_id;                               // DEM实例ID
  std::chrono::system_clock::time_point timestamp;  // 事件发生时间
};

// 前向声明
class DEMSubject;

// 观察者接口（支持优先级和特定事件订阅）
class Observer : public std::enable_shared_from_this<Observer> {
 public:
  // 优先级：1（最高）~10（最低）
  Observer(int priority) : priority_(priority) {}

  // 返回需要订阅的事件类型
  virtual std::vector<DEMStateEvent> getInterestedEvents() const = 0;

  // 处理事件（返回是否成功，用于重试判断）
  virtual bool handleEvent(const std::shared_ptr<DEMSubject>& subject, const DEMEvent& event) = 0;

  // 获取优先级
  int getPriority() const { return priority_; }

  // 观察者名称（用于日志）
  virtual std::string getName() const = 0;

  virtual ~Observer() = default;

 private:
  int priority_;  // 优先级
};

// 被观察者：DEM状态管理器
class DEMSubject : public std::enable_shared_from_this<DEMSubject> {
 public:
  DEMSubject(std::string dem_id, ThreadPool& thread_pool)
      : dem_id_(std::move(dem_id))
      , thread_pool_(thread_pool)
      , current_state_(DEMStateEvent::ActiveToDead) {}

  // 注册观察者（仅订阅其感兴趣的事件）
  void attach(const std::shared_ptr<Observer>& observer) {
    if (!observer) return;

    std::lock_guard<std::mutex> lock(mtx_);
    // 记录观察者感兴趣的事件，避免无意义的通知
    for (auto event : observer->getInterestedEvents()) {
      event_observers_[event].push_back(observer);
    }
  }

  // 移除观察者
  void detach(const std::shared_ptr<Observer>& observer) {
    if (!observer) return;

    std::lock_guard<std::mutex> lock(mtx_);
    for (auto& [event, observers] : event_observers_) {
      auto it =
          std::remove_if(observers.begin(), observers.end(),
                         [&](const std::weak_ptr<Observer>& wp) { return wp.lock() == observer; });
      observers.erase(it, observers.end());
    }
  }

  // 状态变更（触发事件通知）
  void changeState(DEMStateEvent new_event) {
    if (new_event == current_state_) return;  // 状态未变化，无需通知

    DEMEvent event{
        .type = new_event, .dem_id = dem_id_, .timestamp = std::chrono::system_clock::now()};
    current_state_ = new_event;

    // 通知订阅该事件的观察者（按优先级排序）
    notifyObservers(event);
  }

  // 获取当前状态
  DEMStateEvent getCurrentState() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return current_state_;
  }

  std::string getDemID() const { return dem_id_; }

 private:
  // 通知观察者（按优先级排序，异步执行）
  void notifyObservers(const DEMEvent& event) {
    std::vector<std::weak_ptr<Observer>> interested_observers;

    {
      std::lock_guard<std::mutex> lock(mtx_);
      auto it = event_observers_.find(event.type);
      if (it == event_observers_.end()) return;
      interested_observers = it->second;
    }

    // 收集有效观察者并按优先级排序
    std::vector<std::shared_ptr<Observer>> valid_observers;
    for (const auto& wp : interested_observers) {
      if (auto observer = wp.lock()) {
        valid_observers.push_back(observer);
      }
    }

    // 按优先级排序（升序：1优先于2）
    std::sort(valid_observers.begin(), valid_observers.end(),
              [](const std::shared_ptr<Observer>& a, const std::shared_ptr<Observer>& b) {
                return a->getPriority() < b->getPriority();
              });

    // 提交任务到线程池（带优先级）
    for (const auto& observer : valid_observers) {
      auto self = shared_from_this();
      auto event_copy = event;
      // 使用观察者的优先级作为任务优先级
      thread_pool_.enqueue(observer->getPriority(), [observer, self, event_copy]() {
        try {
          observer->handleEvent(self, event_copy);
        } catch (const std::exception& e) {
          std::cerr << "Observer " << observer->getName() << " failed: " << e.what() << std::endl;
        }
      });
    }
  }

 private:
  std::string dem_id_;
  ThreadPool& thread_pool_;
  DEMStateEvent current_state_;
  // 事件-观察者映射：key为事件类型，value为对该事件感兴趣的观察者
  std::unordered_map<DEMStateEvent, std::vector<std::weak_ptr<Observer>>> event_observers_;
  mutable std::mutex mtx_;
};

// 自动重连工具类（模板实现，支持任意可调用对象的重试）
template <typename Func>
class RetryHandler {
 public:
  // 构造：最大重试次数、重试间隔（毫秒）
  RetryHandler(int max_retries, int interval_ms)
      : max_retries_(max_retries), interval_ms_(interval_ms) {}

  // 执行带重试的操作
  bool execute(Func func) {
    int retry_count = 0;
    while (retry_count <= max_retries_) {
      try {
        if (func()) {  // 执行操作，成功则返回
          return true;
        }
        std::cerr << "Operation failed, retrying (" << retry_count << "/" << max_retries_ << ")..."
                  << std::endl;
      } catch (const std::exception& e) {
        std::cerr << "Operation threw exception: " << e.what() << ", retrying (" << retry_count
                  << "/" << max_retries_ << ")..." << std::endl;
      }

      if (retry_count == max_retries_) break;
      std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms_));
      retry_count++;
    }
    std::cerr << "All retries exhausted, operation failed" << std::endl;
    return false;
  }

 private:
  int max_retries_;  // 最大重试次数
  int interval_ms_;  // 重试间隔（毫秒）
};

// 观察者1：连接WebSocket（带自动重连）
class WebSocketObserver : public Observer {
 public:
  WebSocketObserver() : Observer(2) {}  // 优先级2

  std::vector<DEMStateEvent> getInterestedEvents() const override {
    return {DEMStateEvent::DeadToActive};  // 只关注从dead到active的事件
  }

  bool handleEvent(const std::shared_ptr<DEMSubject>& subject, const DEMEvent& event) override {
    std::cout << "[" << getName() << "] 检测到DEM " << subject->getDemID()
              << " 从dead变为active，尝试连接WebSocket..." << std::endl;

    // 模拟WebSocket连接操作（可能失败）
    auto connect_func = [&]() -> bool {
      // 50%概率模拟连接失败
      static std::atomic<int> fail_counter = 0;
      if (fail_counter++ % 2 == 0) {
        throw std::runtime_error("WebSocket connection timeout");
      }
      std::cout << "[" << getName() << "] WebSocket连接成功！" << std::endl;
      return true;
    };

    // 使用自动重连工具（最多重试3次，间隔1秒）
    RetryHandler retry_handler(3, 1000);
    return retry_handler.execute(connect_func);
  }

  std::string getName() const override { return "WebSocketObserver"; }
};

// 观察者2：重新拉起img应用（耗时操作）
class ImgAppObserver : public Observer {
 public:
  ImgAppObserver() : Observer(3) {}  // 优先级3（低于日志和WebSocket）

  std::vector<DEMStateEvent> getInterestedEvents() const override {
    return {DEMStateEvent::DeadToActive};
  }

  bool handleEvent(const std::shared_ptr<DEMSubject>& subject, const DEMEvent& event) override {
    std::cout << "[" << getName() << "] 检测到DEM " << subject->getDemID()
              << " 从dead变为active，开始拉起img应用（耗时操作）..." << std::endl;

    // 模拟耗时操作（3秒）
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "[" << getName() << "] img应用拉起成功！" << std::endl;
    return true;
  }

  std::string getName() const override { return "ImgAppObserver"; }
};

// 观察者3：记录状态变化日志（最高优先级）
class LoggingObserver : public Observer {
 public:
  LoggingObserver() : Observer(1) {}  // 优先级1（最高）

  std::vector<DEMStateEvent> getInterestedEvents() const override {
    // 关注所有状态变化事件
    return {DEMStateEvent::DeadToActive, DEMStateEvent::ActiveToDead, DEMStateEvent::ActiveToIdle,
            DEMStateEvent::IdleToActive};
  }

  bool handleEvent(const std::shared_ptr<DEMSubject>& subject, const DEMEvent& event) override {
    // 格式化时间戳
    auto time = std::chrono::system_clock::to_time_t(event.timestamp);
    std::string time_str = std::ctime(&time);
    time_str.pop_back();  // 移除换行符

    // 日志内容
    std::cout << "[" << time_str << "] [" << getName() << "] DEM " << subject->getDemID()
              << " 状态变化: " << eventTypeToString(event.type) << std::endl;
    return true;
  }

  std::string getName() const override { return "LoggingObserver"; }

 private:
  // 事件类型转字符串（用于日志）
  std::string eventTypeToString(DEMStateEvent type) const {
    switch (type) {
      case DEMStateEvent::DeadToActive:
        return "Dead -> Active";
      case DEMStateEvent::ActiveToDead:
        return "Active -> Dead";
      case DEMStateEvent::ActiveToIdle:
        return "Active -> Idle";
      case DEMStateEvent::IdleToActive:
        return "Idle -> Active";
      default:
        return "Unknown";
    }
  }
};

int main() {
  // 创建线程池（3个工作线程）
  ThreadPool thread_pool(3);

  // 创建被观察者：DEM实例
  auto dem_subject = std::make_shared<DEMSubject>("dem_001", thread_pool);

  // 创建观察者
  auto log_observer = std::make_shared<LoggingObserver>();
  auto ws_observer = std::make_shared<WebSocketObserver>();
  auto img_observer = std::make_shared<ImgAppObserver>();

  // 注册观察者
  dem_subject->attach(log_observer);
  dem_subject->attach(ws_observer);
  dem_subject->attach(img_observer);

  // 模拟DEM状态从dead变为active
  std::cout << "=== 模拟DEM从dead变为active ===" << std::endl;
  dem_subject->changeState(DEMStateEvent::DeadToActive);

  // 等待所有异步任务完成
  std::this_thread::sleep_for(std::chrono::seconds(5));

  // 模拟DEM状态从active变为idle（仅日志观察者会响应）
  std::cout << "\n=== 模拟DEM从active变为idle ===" << std::endl;
  dem_subject->changeState(DEMStateEvent::ActiveToIdle);

  // 等待日志任务完成
  std::this_thread::sleep_for(std::chrono::seconds(1));

  return 0;
}
