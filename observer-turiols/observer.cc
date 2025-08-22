#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

// 前向声明被观察者类
class Subject;

// 观察者接口（抽象基类）
class Observer {
 public:
  // 纯虚函数：更新方法，由具体观察者实现
  virtual void update(Subject* subject) = 0;
  virtual ~Observer() = default;  // 虚析构函数，确保正确析构
};

// 被观察者接口（抽象基类）
class Subject {
 public:
  // 注册观察者
  virtual void attach(Observer* observer) { observers.push_back(observer); }

  // 移除观察者
  virtual void detach(Observer* observer) {
    auto it = std::find(observers.begin(), observers.end(), observer);
    if (it != observers.end()) {
      observers.erase(it);
    }
  }

  // 通知所有观察者
  virtual void notify() {
    for (Observer* observer : observers) {
      observer->update(this);  // 传递当前被观察者指针
    }
  }

  // 获取状态（纯虚函数，由具体被观察者实现）
  virtual int getState() const = 0;
  // 设置状态（纯虚函数，由具体被观察者实现）
  virtual void setState(int state) = 0;

  virtual ~Subject() = default;

 protected:
  std::vector<Observer*> observers;  // 存储观察者列表
};

// 具体被观察者：主题A
class ConcreteSubject : public Subject {
 private:
  int state;  // 被观察者的状态

 public:
  int getState() const override { return state; }

  void setState(int state) override {
    this->state = state;
    notify();  // 状态改变时自动通知所有观察者
  }
};

// 具体观察者：观察者1
class ConcreteObserver1 : public Observer {
 private:
  int ownState;  // 观察者自身的状态
  // 可以存储一个被观察者的引用，方便后续获取状态
  Subject* subject;

 public:
  ConcreteObserver1(Subject* subject) : subject(subject) {
    // 注册到被观察者
    subject->attach(this);
  }
  ~ConcreteObserver1() {
    if (subject != nullptr) {
      subject->detach(this);  // 必须通过被观察者指针操作
    }
  }

  void update(Subject* subject) override {
    // 从被观察者获取状态并更新自身
    ownState = subject->getState();
    std::cout << "Observer1 updated. New state: " << ownState << std::endl;
  }
};

// 具体观察者：观察者2
class ConcreteObserver2 : public Observer {
 private:
  int ownState;
  Subject* subject;

 public:
  ConcreteObserver2(Subject* subject) : subject(subject) { subject->attach(this); }
  ~ConcreteObserver2() {
    if (subject != nullptr) {
      subject->detach(this);  // 必须通过被观察者指针操作
    }
  }

  void update(Subject* subject) override {
    ownState = subject->getState() * 2;  // 观察者2的状态是被观察者状态的2倍
    std::cout << "Observer2 updated. New state: " << ownState << std::endl;
  }
};

int main() {
  // 创建被观察者
  ConcreteSubject* subject = new ConcreteSubject();

  // 创建观察者（会自动注册到被观察者）
  Observer* observer1 = new ConcreteObserver1(subject);
  Observer* observer2 = new ConcreteObserver2(subject);

  std::cout << "设置被观察者状态为 10" << std::endl;
  subject->setState(10);  // 状态改变，自动通知所有观察者

  std::cout << "\n设置被观察者状态为 20" << std::endl;
  subject->setState(20);

  // 移除观察者1
  //   std::cout << "\n移除Observer1后，设置状态为 30" << std::endl;
  //   subject->detach(observer1);
  //   subject->setState(30);  // 此时只有观察者2会收到通知

  // 释放资源
  delete observer1;
  std::cout << "\n设置被观察者状态为 30" << std::endl;
  subject->setState(30);
  delete observer2;
  delete subject;

  return 0;
}
