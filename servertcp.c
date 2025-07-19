#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#define PORT 8888
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

// 线程参数结构体
typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
} thread_args_t;

// 信号处理函数，处理僵尸进程
void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// 处理客户端连接的函数
void handle_client(int client_socket, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int bytes_received;
    
    printf("客户端 %s:%d 已连接 (进程ID: %d, 线程ID: %ld)\n", 
           inet_ntoa(client_addr.sin_addr), 
           ntohs(client_addr.sin_port),
           getpid(),
           pthread_self());
    
    // 发送欢迎消息
    snprintf(response, sizeof(response), 
             "欢迎连接到TCP服务器! (进程ID: %d, 线程ID: %ld)\n", 
             getpid(), pthread_self());
    send(client_socket, response, strlen(response), 0);
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("客户端 %s:%d 断开连接\n", 
                       inet_ntoa(client_addr.sin_addr), 
                       ntohs(client_addr.sin_port));
            } else {
                perror("recv失败");
            }
            break;
        }
        
        buffer[bytes_received] = '\0';
        printf("收到来自 %s:%d 的消息: %s", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port), 
               buffer);
        
        // 检查是否是退出命令
        if (strncmp(buffer, "quit", 4) == 0) {
            printf("客户端 %s:%d 请求断开连接\n", 
                   inet_ntoa(client_addr.sin_addr), 
                   ntohs(client_addr.sin_port));
            break;
        }
        
        // 回显消息
        snprintf(response, sizeof(response), 
                 "服务器回复 (进程ID: %d, 线程ID: %ld): %s", 
                 getpid(), pthread_self(), buffer);
        send(client_socket, response, strlen(response), 0);
    }
    
    close(client_socket);
}

// 线程处理函数
void* thread_handler(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;
    handle_client(args->client_socket, args->client_addr);
    free(args);
    return NULL;
}

// 基础TCP服务器
void basic_server() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    printf("\n=== 启动基础TCP服务器 ===\n");
    
    // 创建socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("创建socket失败");
        return;
    }
    
    // 设置socket选项，允许重用地址
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("设置socket选项失败");
        close(server_socket);
        return;
    }
    
    // 配置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // 绑定socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("绑定失败");
        close(server_socket);
        return;
    }
    
    // 监听连接
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("监听失败");
        close(server_socket);
        return;
    }
    
    printf("基础TCP服务器正在监听端口 %d...\n", PORT);
    printf("注意: 基础服务器一次只能处理一个客户端连接\n");
    
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("接受连接失败");
            continue;
        }
        
        // 处理客户端（阻塞式，一次只能处理一个）
        handle_client(client_socket, client_addr);
    }
    
    close(server_socket);
}

// 多进程服务器
void multiprocess_server() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pid_t pid;
    
    printf("\n=== 启动多进程TCP服务器 ===\n");
    
    // 设置信号处理器处理僵尸进程
    signal(SIGCHLD, sigchld_handler);
    
    // 创建socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("创建socket失败");
        return;
    }
    
    // 设置socket选项
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("设置socket选项失败");
        close(server_socket);
        return;
    }
    
    // 配置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // 绑定socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("绑定失败");
        close(server_socket);
        return;
    }
    
    // 监听连接
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("监听失败");
        close(server_socket);
        return;
    }
    
    printf("多进程TCP服务器正在监听端口 %d...\n", PORT);
    printf("每个客户端连接将创建一个新进程处理\n");
    
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (errno == EINTR) continue; // 被信号中断，继续
            perror("接受连接失败");
            continue;
        }
        
        // 创建子进程处理客户端
        pid = fork();
        if (pid == 0) {
            // 子进程
            close(server_socket); // 子进程不需要监听socket
            handle_client(client_socket, client_addr);
            exit(0);
        } else if (pid > 0) {
            // 父进程
            close(client_socket); // 父进程不需要客户端socket
        } else {
            perror("创建进程失败");
        }
    }
    
    close(server_socket);
}

// 多线程服务器
void multithread_server() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread;
    thread_args_t* args;
    
    printf("\n=== 启动多线程TCP服务器 ===\n");
    
    // 创建socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("创建socket失败");
        return;
    }
    
    // 设置socket选项
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("设置socket选项失败");
        close(server_socket);
        return;
    }
    
    // 配置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // 绑定socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("绑定失败");
        close(server_socket);
        return;
    }
    
    // 监听连接
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("监听失败");
        close(server_socket);
        return;
    }
    
    printf("多线程TCP服务器正在监听端口 %d...\n", PORT);
    printf("每个客户端连接将创建一个新线程处理\n");
    
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("接受连接失败");
            continue;
        }
        
        // 为线程准备参数
        args = malloc(sizeof(thread_args_t));
        if (args == NULL) {
            perror("内存分配失败");
            close(client_socket);
            continue;
        }
        
        args->client_socket = client_socket;
        args->client_addr = client_addr;
        
        // 创建线程处理客户端
        if (pthread_create(&thread, NULL, thread_handler, args) != 0) {
            perror("创建线程失败");
            free(args);
            close(client_socket);
            continue;
        }
        
        // 分离线程，让其自动清理资源
        pthread_detach(thread);
    }
    
    close(server_socket);
}

int main() {
    int choice;
    
    printf("TCP服务器程序\n");
    printf("请选择服务器类型:\n");
    printf("1. 基础TCP服务器 (单线程，一次处理一个客户端)\n");
    printf("2. 多进程TCP服务器 (每个客户端一个进程)\n");
    printf("3. 多线程TCP服务器 (每个客户端一个线程)\n");
    printf("请输入选择 (1-3): ");
    
    if (scanf("%d", &choice) != 1) {
        printf("输入错误\n");
        return 1;
    }
    
    switch (choice) {
        case 1:
            basic_server();
            break;
        case 2:
            multiprocess_server();
            break;
        case 3:
            multithread_server();
            break;
        default:
            printf("无效选择\n");
            return 1;
    }
    
    return 0;
}