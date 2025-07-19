#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define DEFAULT_PORT 8888

void print_usage(const char* program_name) {
    printf("使用方法: %s [服务器IP] [端口]\n", program_name);
    printf("示例:\n");
    printf("  %s                    # 连接到 localhost:8888\n", program_name);
    printf("  %s 192.168.1.100      # 连接到 192.168.1.100:8888\n", program_name);
    printf("  %s 192.168.1.100 9999 # 连接到 192.168.1.100:9999\n", program_name);
}

int main(int argc, char* argv[]) {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    int bytes_received;
    char* server_ip = "127.0.0.1"; // 默认本地地址
    int server_port = DEFAULT_PORT;
    
    // 解析命令行参数
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        server_ip = argv[1];
    }
    
    if (argc > 2) {
        server_port = atoi(argv[2]);
        if (server_port <= 0 || server_port > 65535) {
            printf("错误: 端口号必须在 1-65535 之间\n");
            return 1;
        }
    }
    
    printf("TCP客户端程序\n");
    printf("正在连接到服务器 %s:%d...\n", server_ip, server_port);
    
    // 创建socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("创建socket失败");
        return 1;
    }
    
    // 配置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    // 将IP地址从字符串转换为网络地址
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        printf("错误: 无效的IP地址 %s\n", server_ip);
        close(client_socket);
        return 1;
    }
    
    // 连接到服务器
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("连接失败: %s\n", strerror(errno));
        printf("请确保:\n");
        printf("1. 服务器正在运行\n");
        printf("2. IP地址和端口正确\n");
        printf("3. 网络连接正常\n");
        close(client_socket);
        return 1;
    }
    
    printf("成功连接到服务器!\n");
    
    // 接收服务器的欢迎消息
    memset(buffer, 0, BUFFER_SIZE);
    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("服务器消息: %s", buffer);
    }
    
    printf("\n使用说明:\n");
    printf("- 输入消息并按回车发送\n");
    printf("- 输入 'quit' 退出程序\n");
    printf("- 按 Ctrl+C 强制退出\n");
    printf("========================================\n");
    
    // 主循环：发送和接收消息
    while (1) {
        printf("请输入消息: ");
        fflush(stdout);
        
        // 读取用户输入
        if (fgets(message, sizeof(message), stdin) == NULL) {
            printf("\n读取输入失败或收到EOF信号，退出...\n");
            break;
        }
        
        // 检查是否为退出命令
        if (strncmp(message, "quit", 4) == 0) {
            printf("正在断开连接...\n");
            send(client_socket, message, strlen(message), 0);
            break;
        }
        
        // 发送消息到服务器
        if (send(client_socket, message, strlen(message), 0) < 0) {
            perror("发送消息失败");
            break;
        }
        
        // 接收服务器回复
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("服务器关闭了连接\n");
            } else {
                perror("接收消息失败");
            }
            break;
        }
        
        buffer[bytes_received] = '\0';
        printf("服务器回复: %s", buffer);
        printf("----------------------------------------\n");
    }
    
    // 关闭socket
    close(client_socket);
    printf("客户端已关闭\n");
    
    return 0;
}