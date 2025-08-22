#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

// 前向声明
class Subject;

// 观察者接口（抽象基类）
class Observer : public std::enable_shared_from_this<Observer> {
 public:
  // 纯虚函数：接收通知（返回是否处理成功，用于异常反馈）
  virtual bool onUpdate(const std::shared_ptr<Subject>& subject) = 0;

  // 虚析构函数（确保子类正确析构）
  virtual ~Observer() = default;
};

// 被观察者接口（抽象基类）
class Subject : public std::enable_shared_from_this<Subject> {
 public:
  // 注册观察者（线程安全）
  void attach(const std::shared_ptr<Observer>& observer) {
    if (!observer) {
      throw std::invalid_argument("Observer cannot be null");
    }
    std::lock_guard<std::mutex> lock(mtx);  // 加锁保护
    observers.push_back(observer);
  }

  // 移除观察者（线程安全）
  void detach(const std::shared_ptr<Observer>& observer) {
    if (!observer) {
      return;
    }
    std::lock_guard<std::mutex> lock(mtx);
    // 用weak_ptr比较（避免shared_ptr的强引用影响生命周期）
    auto it =
        std::remove_if(observers.begin(), observers.end(),
                       [&](const std::weak_ptr<Observer>& wp) { return wp.lock() == observer; });
    observers.erase(it, observers.end());
  }

  // 通知所有观察者（线程安全，异常安全）
  void notify() {
    std::vector<std::weak_ptr<Observer>> currentObservers;
    {
      // 拷贝当前观察者列表（避免通知时列表被修改）
      std::lock_guard<std::mutex> lock(mtx);
      currentObservers = observers;
    }

    // 遍历并通知（每个观察者的异常不影响其他）
    for (const auto& wp : currentObservers) {
      // 用lock()获取shared_ptr，检查观察者是否还存在
      if (auto observer = wp.lock()) {
        try {
          // 传递被观察者的shared_ptr（确保通知期间被观察者不被销毁）
          if (!observer->onUpdate(shared_from_this())) {
            std::cerr << "Observer update failed, but continues..." << std::endl;
          }
        } catch (const std::exception& e) {
          // 捕获单个观察者的异常，不中断整个通知流程
          std::cerr << "Observer threw exception: " << e.what() << ", continues..." << std::endl;
        } catch (...) {
          std::cerr << "Observer threw unknown exception, continues..." << std::endl;
        }
      }
    }
  }

  // 获取被观察者标识（用于区分不同主题）
  virtual std::string getID() const = 0;

  // 获取状态（纯虚函数）
  virtual int getState() const = 0;

  // 设置状态并触发通知
  virtual void setState(int state) = 0;

  virtual ~Subject() = default;

 protected:
  // 用weak_ptr存储观察者：避免与观察者的shared_ptr形成循环引用
  std::vector<std::weak_ptr<Observer>> observers;
  mutable std::mutex mtx;  // 线程安全锁（mutable允许const函数中加锁）
};

// 具体被观察者1：温度传感器
class TemperatureSensor : public Subject {
 private:
  int temperature;
  const std::string id;

 public:
  TemperatureSensor(std::string id) : id(std::move(id)), temperature(0) {}

  std::string getID() const override { return id; }
  int getState() const override { return temperature; }

  void setState(int state) override {
    if (state < -50 || state > 100) {  // 模拟参数校验
      throw std::out_of_range("Temperature out of range (-50~100)");
    }
    temperature = state;
    notify();  // 状态变化触发通知
  }
};

// 具体被观察者2：湿度传感器
class HumiditySensor : public Subject {
 private:
  int humidity;
  const std::string id;

 public:
  HumiditySensor(std::string id) : id(std::move(id)), humidity(0) {}

  std::string getID() const override { return id; }
  int getState() const override { return humidity; }

  void setState(int state) override {
    if (state < 0 || state > 100) {
      throw std::out_of_range("Humidity out of range (0~100)");
    }
    humidity = state;
    notify();
  }
};

// 具体观察者：监控中心（同时观察温度和湿度）
class MonitoringCenter : public Observer {
 private:
  std::string name;

 public:
  MonitoringCenter(std::string name) : name(std::move(name)) {}

  bool onUpdate(const std::shared_ptr<Subject>& subject) override {
    if (!subject) {
      return false;
    }

    // 根据被观察者ID区分处理
    if (subject->getID().find("temperature") != std::string::npos) {
      std::cout << "[" << name << "] 温度更新: " << subject->getState() << "℃" << std::endl;
      // 模拟偶尔出现的异常
      if (subject->getState() > 80) {
        throw std::runtime_error("High temperature alert!");
      }
    } else if (subject->getID().find("humidity") != std::string::npos) {
      std::cout << "[" << name << "] 湿度更新: " << subject->getState() << "%" << std::endl;
    }
    return true;
  }
};

int main() {
  // 创建被观察者（智能指针管理生命周期）
  auto tempSensor = std::make_shared<TemperatureSensor>("temperature_sensor_1");
  auto humiSensor = std::make_shared<HumiditySensor>("humidity_sensor_1");

  // 创建观察者（智能指针管理）
  auto monitor1 = std::make_shared<MonitoringCenter>("Monitor-A");
  auto monitor2 = std::make_shared<MonitoringCenter>("Monitor-B");

  // 建立N对N关系：
  // - monitor1观察两个传感器
  // - monitor2只观察温度传感器
  tempSensor->attach(monitor1);
  humiSensor->attach(monitor1);
  tempSensor->attach(monitor2);

  try {
    // 触发状态更新
    std::cout << "=== 设置温度为 25℃ ===" << std::endl;
    tempSensor->setState(25);

    std::cout << "\n=== 设置湿度为 60% ===" << std::endl;
    humiSensor->setState(60);

    std::cout << "\n=== 设置温度为 85℃（会触发异常） ===" << std::endl;
    tempSensor->setState(85);  // 此处会导致monitor1抛出异常

    std::cout << "\n=== 设置湿度为 70% ===" << std::endl;
    humiSensor->setState(70);  // 异常后仍能正常通知
  } catch (const std::exception& e) {
    std::cerr << "Main caught exception: " << e.what() << std::endl;
  }

  // 智能指针自动管理析构，无需手动delete
  return 0;
}
