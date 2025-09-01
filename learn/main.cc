#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define SIZE 4096
#define MAX_CONTENT_SIZE (SIZE - sizeof(int) - sizeof(int))  // 预留控制字段

// 共享内存结构
struct shared_data {
    int operation_flag;  // 0=写入, 1=追加, 2=清空, 3=结束
    int content_length;  // 当前内容长度
    char content[MAX_CONTENT_SIZE];
};

int main() {
    // 创建共享匿名映射
    struct shared_data *shared = (struct shared_data*)mmap(NULL, SIZE, PROT_READ | PROT_WRITE, 
                                                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared == MAP_FAILED) { perror("mmap"); exit(EXIT_FAILURE); }

    // 初始化共享内存
    memset(shared, 0, sizeof(struct shared_data));
    
    pid_t pid = fork();
    if (pid == -1) { perror("fork"); exit(EXIT_FAILURE); }

    if (pid == 0) {
        // === 子进程：复杂的写入操作 ===
        printf("=== 子进程开始工作 ===\n");
        
        // 第1次：初始写入
        strcpy(shared->content, "初始消息");
        shared->content_length = strlen(shared->content);
        shared->operation_flag = 0;  // 写入完成
        printf("子进程: 初始写入 -> '%s'\n", shared->content);
        sleep(1);
        
        // 第2次：追加内容
        char append_msg[] = " + 追加内容1";
        if (shared->content_length + strlen(append_msg) < MAX_CONTENT_SIZE) {
            strcat(shared->content, append_msg);
            shared->content_length = strlen(shared->content);
            shared->operation_flag = 1;  // 追加完成
            printf("子进程: 追加后 -> '%s'\n", shared->content);
        }
        sleep(1);
        
        // 第3次：再次追加
        char append_msg2[] = " + 追加内容2";
        if (shared->content_length + strlen(append_msg2) < MAX_CONTENT_SIZE) {
            strcat(shared->content, append_msg2);
            shared->content_length = strlen(shared->content);
            shared->operation_flag = 1;  // 追加完成
            printf("子进程: 再次追加 -> '%s'\n", shared->content);
        }
        sleep(1);
        
        // 第4次：清空并重新写入
        memset(shared->content, 0, MAX_CONTENT_SIZE);
        strcpy(shared->content, "清空后的新消息");
        shared->content_length = strlen(shared->content);
        shared->operation_flag = 2;  // 清空重写完成
        printf("子进程: 清空重写 -> '%s'\n", shared->content);
        sleep(1);
        
        // 第5次：最终追加
        char final_msg[] = " [最终追加]";
        if (shared->content_length + strlen(final_msg) < MAX_CONTENT_SIZE) {
            strcat(shared->content, final_msg);
            shared->content_length = strlen(shared->content);
            shared->operation_flag = 1;  // 追加完成
            printf("子进程: 最终状态 -> '%s'\n", shared->content);
        }
        
        // 通知父进程结束
        shared->operation_flag = 3;
        printf("=== 子进程工作完成 ===\n");
        
    } else {
        // === 父进程：监控和读取 ===
        printf("=== 父进程开始监控 ===\n");
        
        int last_flag = -1;
        int operation_count = 0;
        
        while (1) {
            // 检查操作标志变化
            if (shared->operation_flag != last_flag) {
                operation_count++;
                
                switch (shared->operation_flag) {
                    case 0:
                        printf("父进程[%d]: 检测到写入操作 -> '%s' (长度: %d)\n", 
                               operation_count, shared->content, shared->content_length);
                        break;
                    case 1:
                        printf("父进程[%d]: 检测到追加操作 -> '%s' (长度: %d)\n", 
                               operation_count, shared->content, shared->content_length);
                        break;
                    case 2:
                        printf("父进程[%d]: 检测到清空重写 -> '%s' (长度: %d)\n", 
                               operation_count, shared->content, shared->content_length);
                        break;
                    case 3:
                        printf("父进程[%d]: 子进程完成所有操作 -> 最终内容: '%s'\n", 
                               operation_count, shared->content);
                        goto cleanup;
                }
                last_flag = shared->operation_flag;
            }
            usleep(100000);  // 100ms 轮询
        }
        
cleanup:
        wait(NULL);  // 等待子进程结束
        printf("=== 父进程监控完成 ===\n");
        printf("总共监控到 %d 次操作\n", operation_count);
    }

    if (munmap(shared, SIZE) == -1) { perror("munmap"); }
    return 0;
}