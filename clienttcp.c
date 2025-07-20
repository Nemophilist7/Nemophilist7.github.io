#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#define BUFFER_SIZE 1024
#define DEFAULT_PORT 8888

// 全局变量，用于信号处理
volatile sig_atomic_t keep_running = 1;
int global_socket = -1;

// 信号处理函数
void signal_handler(int sig) {
    (void)sig;
    keep_running = 0;
    if (global_socket != -1) {
        close(global_socket);
    }
    printf("\n👋 收到中断信号，正在退出...\n");
    exit(0);
}

void print_usage(const char* program_name) {
    printf("🌐 TCP客户端程序 (跨机器版本)\n");
    printf("=======================================\n");
    printf("使用方法: %s [服务器IP] [端口]\n", program_name);
    printf("\n示例:\n");
    printf("  %s                        # 连接到本机 127.0.0.1:8888\n", program_name);
    printf("  %s 192.168.1.100          # 连接到 192.168.1.100:8888\n", program_name);
    printf("  %s 192.168.1.100 9999     # 连接到 192.168.1.100:9999\n", program_name);
    printf("  %s 10.0.0.5 8888          # 连接到 10.0.0.5:8888\n", program_name);
    printf("\n常用内网IP范围:\n");
    printf("  192.168.x.x  (家庭/办公网络)\n");
    printf("  10.x.x.x     (企业网络)\n");
    printf("  172.16-31.x.x (企业网络)\n");
    printf("\n💡 提示:\n");
    printf("  - 确保目标机器上的服务器程序正在运行\n");
    printf("  - 检查防火墙设置是否允许连接\n");
    printf("  - 使用 ping IP地址 测试网络连通性\n");
}

// 测试网络连通性
int test_connectivity(const char* server_ip, int server_port) {
    printf("🔍 测试网络连通性...\n");
    
    // 创建测试socket
    int test_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (test_socket == -1) {
        printf("❌ 创建测试socket失败: %s\n", strerror(errno));
        return 0;
    }
    
    // 设置连接超时
    struct timeval timeout;
    timeout.tv_sec = 5;  // 5秒超时
    timeout.tv_usec = 0;
    setsockopt(test_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(test_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    struct sockaddr_in test_addr;
    memset(&test_addr, 0, sizeof(test_addr));
    test_addr.sin_family = AF_INET;
    test_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_ip, &test_addr.sin_addr) <= 0) {
        printf("❌ IP地址格式错误: %s\n", server_ip);
        close(test_socket);
        return 0;
    }
    
    printf("   正在尝试连接 %s:%d...\n", server_ip, server_port);
    
    if (connect(test_socket, (struct sockaddr*)&test_addr, sizeof(test_addr)) < 0) {
        printf("❌ 连接测试失败: %s\n", strerror(errno));
        printf("\n🔧 可能的解决方案:\n");
        printf("   1. 检查服务器是否正在运行\n");
        printf("   2. 验证IP地址和端口是否正确\n");
        printf("   3. 检查防火墙设置\n");
        printf("   4. 确认网络连通性: ping %s\n", server_ip);
        close(test_socket);
        return 0;
    }
    
    printf("✅ 网络连通性测试通过\n");
    close(test_socket);
    return 1;
}

// 显示连接信息
void show_connection_info(const char* server_ip, int server_port) {
    printf("\n📡 连接信息\n");
    printf("===================\n");
    printf("目标服务器: %s:%d\n", server_ip, server_port);
    
    // 显示本地时间
    time_t now = time(NULL);
    printf("连接时间: %s", ctime(&now));
    
    // 显示本机IP（简单版本）
    printf("本机IP: ");
    system("hostname -I | awk '{print $1}' | tr -d '\\n'");
    printf("\n");
    
    printf("===================\n");
}

// 交互式获取服务器信息
void get_server_info_interactive(char** server_ip, int* server_port) {
    static char ip_buffer[256];
    char port_buffer[16];
    
    printf("\n🔧 交互式连接配置\n");
    printf("=======================\n");
    
    // 获取IP地址
    printf("请输入服务器IP地址 (回车使用默认 127.0.0.1): ");
    fflush(stdout);
    
    if (fgets(ip_buffer, sizeof(ip_buffer), stdin) != NULL) {
        // 移除换行符
        ip_buffer[strcspn(ip_buffer, "\n")] = 0;
        
        if (strlen(ip_buffer) == 0) {
            strcpy(ip_buffer, "127.0.0.1");
        }
        *server_ip = ip_buffer;
    }
    
    // 获取端口
    printf("请输入服务器端口 (回车使用默认 %d): ", DEFAULT_PORT);
    fflush(stdout);
    
    if (fgets(port_buffer, sizeof(port_buffer), stdin) != NULL) {
        port_buffer[strcspn(port_buffer, "\n")] = 0;
        
        if (strlen(port_buffer) == 0) {
            *server_port = DEFAULT_PORT;
        } else {
            int port = atoi(port_buffer);
            if (port > 0 && port <= 65535) {
                *server_port = port;
            } else {
                printf("⚠️  端口无效，使用默认端口 %d\n", DEFAULT_PORT);
                *server_port = DEFAULT_PORT;
            }
        }
    }
    
    printf("✅ 配置完成: %s:%d\n", *server_ip, *server_port);
}

int main(int argc, char* argv[]) {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    int bytes_received;
    char* server_ip = "127.0.0.1";
    int server_port = DEFAULT_PORT;
    int interactive_mode = 0;
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("🌐 TCP客户端程序 (跨机器版本)\n");
    printf("=======================================\n");
    printf("版本: 1.1 - 增强跨机器连接功能\n");
    printf("=======================================\n");
    
    // 解析命令行参数
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--interactive") == 0) {
            interactive_mode = 1;
        } else {
            server_ip = argv[1];
        }
    }
    
    if (argc > 2 && !interactive_mode) {
        server_port = atoi(argv[2]);
        if (server_port <= 0 || server_port > 65535) {
            printf("❌ 错误: 端口号必须在 1-65535 之间\n");
            return 1;
        }
    }
    
    // 交互式模式
    if (interactive_mode || argc == 1) {
        get_server_info_interactive(&server_ip, &server_port);
    }
    
    // 显示连接信息
    show_connection_info(server_ip, server_port);
    
    // 测试网络连通性
    if (!test_connectivity(server_ip, server_port)) {
        printf("\n❌ 连接前测试失败，程序退出\n");
        printf("💡 提示: 使用 %s -h 查看帮助信息\n", argv[0]);
        return 1;
    }
    
    printf("\n🚀 正在建立连接...\n");
    
    // 创建socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        printf("❌ 创建socket失败: %s\n", strerror(errno));
        return 1;
    }
    
    global_socket = client_socket;
    
    // 配置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        printf("❌ 错误: 无效的IP地址 %s\n", server_ip);
        close(client_socket);
        return 1;
    }
    
    // 连接到服务器
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("❌ 连接失败: %s\n", strerror(errno));
        printf("\n🔧 故障排除建议:\n");
        printf("1. 确保服务器程序正在运行\n");
        printf("2. 检查IP地址: %s\n", server_ip);
        printf("3. 检查端口: %d\n", server_port);
        printf("4. 测试网络连通性: ping %s\n", server_ip);
        printf("5. 检查防火墙设置\n");
        printf("6. 确认服务器在正确的网络接口上监听\n");
        close(client_socket);
        return 1;
    }
    
    printf("✅ 成功连接到服务器 %s:%d!\n", server_ip, server_port);
    
    // 接收服务器的欢迎消息
    memset(buffer, 0, BUFFER_SIZE);
    bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("\n📨 服务器欢迎消息:\n");
        printf("─────────────────────────\n");
        printf("%s", buffer);
        printf("─────────────────────────\n");
    }
    
    printf("\n💬 进入聊天模式\n");
    printf("===============================\n");
    printf("📝 使用说明:\n");
    printf("  - 输入消息并按回车发送\n");
    printf("  - 输入 'quit' 正常退出\n");
    printf("  - 按 Ctrl+C 强制退出\n");
    printf("  - 输入 'help' 显示帮助\n");
    printf("  - 输入 'info' 显示连接信息\n");
    printf("===============================\n\n");
    
    // 主循环：发送和接收消息
    while (keep_running) {
        printf("💭 请输入消息: ");
        fflush(stdout);
        
        // 读取用户输入
        if (fgets(message, sizeof(message), stdin) == NULL) {
            printf("\n📥 读取输入失败或收到EOF信号，退出...\n");
            break;
        }
        
        // 处理特殊命令
        if (strncmp(message, "help", 4) == 0) {
            printf("\n📋 可用命令:\n");
            printf("  quit - 退出程序\n");
            printf("  help - 显示此帮助\n");
            printf("  info - 显示连接信息\n");
            printf("  其他 - 发送到服务器\n\n");
            continue;
        }
        
        if (strncmp(message, "info", 4) == 0) {
            show_connection_info(server_ip, server_port);
            continue;
        }
        
        // 检查是否为退出命令
        if (strncmp(message, "quit", 4) == 0) {
            printf("👋 正在断开连接...\n");
            send(client_socket, message, strlen(message), 0);
            break;
        }
        
        // 发送消息到服务器
        if (send(client_socket, message, strlen(message), 0) < 0) {
            printf("❌ 发送消息失败: %s\n", strerror(errno));
            printf("🔄 连接可能已断开，尝试重新连接或退出程序\n");
            break;
        }
        
        // 接收服务器回复
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("🔌 服务器关闭了连接\n");
            } else {
                printf("❌ 接收消息失败: %s\n", strerror(errno));
            }
            printf("💡 连接已断开，程序退出\n");
            break;
        }
        
        buffer[bytes_received] = '\0';
        printf("📨 服务器回复: %s", buffer);
        printf("─────────────────────────────────────\n");
    }
    
    // 关闭socket
    close(client_socket);
    printf("\n👋 客户端已关闭\n");
    printf("感谢使用 TCP 客户端程序！\n");
    
    return 0;
}