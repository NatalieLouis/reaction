#include <mqueue.h>
#include <fcntl.h>        // 为 O_CREAT, O_RDWR 等
#include <sys/stat.h>     // 为权限常量
#include <string.h>       // 为 strlen
#include <cstdlib>        // 为 exit
#include <unistd.h>       // 为 unistd 函数
#include <thread>
#include <iostream>
#include <chrono>
#include <string>         // 为 std::string 和 std::to_string

class MultiMessageDemo {
private:
    const char* queue_name = "/multi_message_queue";
    mqd_t mq_sender, mq_receiver;
    
public:
    MultiMessageDemo() {
        // 设置队列属性
        struct mq_attr attr;
        attr.mq_flags = 0;
        attr.mq_maxmsg = 10;     // 可以存储10条消息（系统限制）
        attr.mq_msgsize = 256;   // 每条消息最大256字节
        attr.mq_curmsgs = 0;
        
        
        // 创建/打开队列（生产者）
        mq_sender = mq_open(queue_name, O_CREAT | O_WRONLY, 0666, &attr);
        // 打开队列（消费者） 
        mq_receiver = mq_open(queue_name, O_RDONLY);
        
        if (mq_sender == (mqd_t)-1 || mq_receiver == (mqd_t)-1) {
            perror("mq_open failed");
            exit(1);
        }
    }
    
    // 生产者：发送多条消息
    void producer() {
        std::cout << "=== 生产者开始工作 ===\n";
        
        for (int i = 1; i <= 15; ++i) {
            std::string message = "消息#" + std::to_string(i) + " [重要程度:" + std::to_string(i % 3) + "]";
            
            // 发送消息（带优先级）
            unsigned int priority = i % 3;  // 0-2的优先级
            if (mq_send(mq_sender, message.c_str(), message.length(), priority) == 0) {
                std::cout << "✓ 发送: " << message << " (优先级:" << priority << ")\n";
            } else {
                perror("发送失败");
            }
            
            // 模拟生产间隔
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        // 发送结束标志
        mq_send(mq_sender, "END", 3, 10);  // 最高优先级
        std::cout << "=== 生产者完成，发送了15条消息 ===\n";
    }
    
    // 消费者：持续消费消息
    void consumer() {
        std::cout << "=== 消费者开始工作 ===\n";
        
        char buffer[256];
        unsigned int priority;
        int message_count = 0;
        
        while (true) {
            // 接收消息（阻塞模式）
            ssize_t bytes_read = mq_receive(mq_receiver, buffer, sizeof(buffer), &priority);
            
            if (bytes_read >= 0) {
                buffer[bytes_read] = '\0';
                std::string received_msg(buffer);
                
                // 检查结束标志
                if (received_msg == "END") {
                    std::cout << "🏁 收到结束信号，消费者退出\n";
                    break;
                }
                
                message_count++;
                std::cout << "✓ [" << message_count << "] 接收: " << received_msg 
                         << " (优先级:" << priority << ")\n";
                
                // 模拟消费处理时间
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            } else {
                perror("接收失败");
                break;
            }
        }
        
        std::cout << "=== 消费者完成，总共处理了 " << message_count << " 条消息 ===\n";
    }
    
    // 检查队列状态
    void check_queue_status() {
        struct mq_attr attr;
        if (mq_getattr(mq_receiver, &attr) == 0) {
            std::cout << "📊 队列状态:\n";
            std::cout << "   - 当前消息数: " << attr.mq_curmsgs << "\n";
            std::cout << "   - 最大消息数: " << attr.mq_maxmsg << "\n";
            std::cout << "   - 消息大小: " << attr.mq_msgsize << " 字节\n";
        }
    }
    
    ~MultiMessageDemo() {
        mq_close(mq_sender);
        mq_close(mq_receiver);
        mq_unlink(queue_name);  // 删除队列
    }
};

int main() {
    MultiMessageDemo demo;
    
    // 启动生产者和消费者线程
    std::thread producer_thread(&MultiMessageDemo::producer, &demo);
    std::thread consumer_thread(&MultiMessageDemo::consumer, &demo);
    
    // 等待1秒后检查队列状态
    std::this_thread::sleep_for(std::chrono::seconds(1));
    demo.check_queue_status();
    
    // 等待线程完成
    producer_thread.join();
    consumer_thread.join();
    
    return 0;
}