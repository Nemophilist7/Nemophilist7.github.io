CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread
TARGET_SERVER = servertcp
TARGET_CLIENT = clienttcp
SOURCE_SERVER = servertcp.c
SOURCE_CLIENT = clienttcp.c

# 默认目标：编译所有程序
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# 编译服务器程序
$(TARGET_SERVER): $(SOURCE_SERVER)
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) $(SOURCE_SERVER)

# 编译客户端程序
$(TARGET_CLIENT): $(SOURCE_CLIENT)
	$(CC) $(CFLAGS) -o $(TARGET_CLIENT) $(SOURCE_CLIENT)

# 清理编译生成的文件
clean:
	rm -f $(TARGET_SERVER) $(TARGET_CLIENT)

# 安装（复制到系统路径，需要sudo权限）
install: all
	sudo cp $(TARGET_SERVER) /usr/local/bin/
	sudo cp $(TARGET_CLIENT) /usr/local/bin/

# 卸载
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET_SERVER)
	sudo rm -f /usr/local/bin/$(TARGET_CLIENT)

# 运行服务器（基础版本）
run-server: $(TARGET_SERVER)
	./$(TARGET_SERVER)

# 运行客户端（连接本地服务器）
run-client: $(TARGET_CLIENT)
	./$(TARGET_CLIENT)

# 帮助信息
help:
	@echo "可用的make目标:"
	@echo "  all          - 编译所有程序（默认）"
	@echo "  servertcp    - 只编译服务器程序"
	@echo "  clienttcp    - 只编译客户端程序"
	@echo "  clean        - 清理编译生成的文件"
	@echo "  install      - 安装程序到系统路径（需要sudo）"
	@echo "  uninstall    - 从系统路径卸载程序（需要sudo）"
	@echo "  run-server   - 编译并运行服务器"
	@echo "  run-client   - 编译并运行客户端"
	@echo "  help         - 显示此帮助信息"

# 声明伪目标
.PHONY: all clean install uninstall run-server run-client help